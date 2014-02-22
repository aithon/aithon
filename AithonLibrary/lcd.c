#include "Aithon.h"

// LCD Commands
#define _CLEAR_CMD      0x01
#define _ON_CMD         0x0C
#define _OFF_CMD        0x08
#define _FUNC_SET_CMD   0x38
#define _ENTRY_MODE_CMD 0x06

// Other Constants
#define _HOME_ADDR      0x80
#define _LINE_INCR      0x40

// LCD Pin Control Macros
#define _SET_CLK() palSetPad(GPIOA, GPIOA_LCD_CLK)
#define _CLR_CLK() palClearPad(GPIOA, GPIOA_LCD_CLK)
#define _SET_DATA() palSetPad(GPIOA, GPIOA_LCD_DATA)
#define _CLR_DATA() palClearPad(GPIOA, GPIOA_LCD_DATA)
#define _SET_EN() palSetPad(GPIOD, GPIOD_LCD_E)
#define _CLR_EN() palClearPad(GPIOD, GPIOD_LCD_E)
#define _SET_RS() palSetPad(GPIOD, GPIOD_LCD_RS)
#define _CLR_RS() palClearPad(GPIOD, GPIOD_LCD_RS)


static WORKING_AREA(scrollingThreadWorkingArea, 128);
Mutex _scrollMtx;
int _scrollRow = 0, _scrollCol = 0;
int _scrollDelay = 300; //scrolling delay in ms
int _scrollWidth = 16; //width of scrolling area
int _scrollMsgLen = 0; //length of message to scroll (not counting terminating null)
int _scrollStatus = 1;  //1=scrolling, 2=paused, 0=exit scrolling thread
char _scrollBuf[100];


BaseSequentialStream LCD;
static msg_t _put(void *instance, uint8_t b)
{
   (void)instance;
   lcd_printChar(b);
   return RDY_OK;
}
static struct BaseSequentialStreamVMT _vmt = {NULL, NULL, _put, NULL};


// Writes a byte of data to the LCD.
void _write_byte(uint8_t data)
{
	//set the LCD's E (Enable) line high, so it can fall later
   _SET_EN();
	//write the data to the shift register
   int i;
   for (i = 0; i < 8; i++)
   {
      _CLR_CLK();
      if (data & (0x80 >> i))
         _SET_DATA();
      else
         _CLR_DATA();
      _SET_CLK();
      delayUs(1);
   }
	//set the LCD's E (Enable) line low to latch in the data
   _CLR_EN();
}

void _write_data(uint8_t data)
{
	//set RS (Register Select) line high to select data register
   _SET_RS();
   _write_byte(data);
   delayUs(50);
}

// Writes a command byte to the LCD.
void _write_control(uint8_t data)
{
	//set RS (Register Select) line low to select command register
   _CLR_RS();
	_write_byte(data);
	//wait for the instruction to be executed
	delayUs(100);
}

// Initializes the LCD as described in the HD44780 datasheet.
void _lcd_init(void)
{
   LCD.vmt = &_vmt;
  
	//set LCD E (Enable) line low inititally, so it can rise later
   _CLR_EN();

	//wait 15ms after power on
	delayMs(15);

	//Issue 'Function Set' commands to initialize LCD for 8-bit interface mode
	_write_control(_FUNC_SET_CMD);
	delayUs(4900); //+100us in _write_control = 5000us or 5ms total
	_write_control(_FUNC_SET_CMD);
	delayUs(50); //+100us in _write_control = 150us total
	_write_control(_FUNC_SET_CMD);

	//Function Set command to specify 2 display lines and character font
	_write_control(_FUNC_SET_CMD);

	//Display off
	lcd_off();

	//Clear display
	lcd_clear();

	//Set entry mode
	_write_control(_ENTRY_MODE_CMD);

	//Display on
	lcd_on();
}


// Clears all characters on the display and resets the cursor to the home position.
void lcd_clear(void)
{
	_write_control(_CLEAR_CMD);
	delayUs(3300);
}

// Shows the characters on the screen, if they were hidden with lcdOff().
void lcd_on(void)
{
	_write_control(_ON_CMD);
}

// Hides the characters on the screen. Can be unhidden again with lcdOn().
void lcd_off(void)
{
	_write_control(_OFF_CMD);
}

// Sets the cursor position on the screen.
void lcd_cursor(uint8_t col, uint8_t row)
{
   if (col >= 16 || row >= 2)
   {
      return;
   }
   
   uint8_t addr = _HOME_ADDR + row * _LINE_INCR + col;
   _write_control(addr);
}

// Prints a character at the current cursor position on the screen.
void lcd_printChar(char data)
{
   if (data == '\n')
   {
      lcd_cursor(0, 1);
      return;
   }
   _write_data(data);
}



//LCD scrolling thread
static msg_t scrollingThread(void *arg) 
{
   (void)arg;
   int cur_start;
   int i;
   int times;
   
   cur_start = 0;

   while (_scrollStatus != 0) 
   {
      if (_scrollStatus == 1) 
      {
         chMtxLock(&_scrollMtx);
         //set cursor position
         lcd_cursor(_scrollCol, _scrollRow);

         i=cur_start;
         times=0;

         //print the message
         while (times < _scrollWidth)
         {
            lcd_printChar(_scrollBuf[i]);
            i++;
            times++;

            if (i == _scrollMsgLen)
               i = i % _scrollMsgLen;
         }

         chMtxUnlock();
         //adjust the scroll position
         cur_start++;
         if (cur_start == _scrollMsgLen) 
            cur_start = 0;
      }

      chThdSleepMilliseconds(_scrollDelay);
   }

   return 0;
}

//call this after calling scrollMessage
void scroll_init(void) 
{
   chMtxInit(&_scrollMtx);
   //start the scrolling thread
   (void)chThdCreateStatic(scrollingThreadWorkingArea, sizeof(scrollingThreadWorkingArea),
         NORMALPRIO, scrollingThread, NULL);

}

//set starting position of scroll message on the LCD
void scrollSetCursor(int row, int col) 
{
   _scrollRow = row;
   _scrollCol = col;
}

void scrollSetDelay(int delay) {
   _scrollDelay = delay;
}

//enable scrolling
void scrollEnable(void) {
   _scrollStatus = 1;
}

//pause scrolling
void scrollPause(void) {
   _scrollStatus = 2;
}

//end scrolling thread
void scrollExit(void) {
   _scrollStatus = 0;
}

//scrollMessage
//
//Call this first to setup the message, location, and scrolling width.
//Then call scroll_init to start the scrolling thread.
//
//m = message
//row = starting row 
//col = starting col
//window = size the scrolling area on the LCD (16=a full line)
void scrollMessage(char* m, int row, int col, int window) 
{
   int i = 0;
   int j;

   //copy string
   while(m[i] != 0)
   {
      _scrollBuf[i] = m[i];
      i++;
   }
   
   //add a few blank spaces at the end of the message
   for (j=0; j<3; j++) 
   {
      _scrollBuf[i] = ' ';
      i++;
   }

   _scrollBuf[i] = m[i]; //copy null
   _scrollMsgLen = i; //message length not counting the null
   scrollSetCursor(row, col);

   if ((col + window) > 16) 
   {
      window = 16-col;
   }

   _scrollWidth = window;
}