#include "Aithon.h"

#define HOME         0x80
#define SECOND_LINE  0XC0

BaseSequentialStream LCD;
static msg_t putt(void *instance, uint8_t b)
{
  (void)instance;
  aiLCDPrintChar(b);
  return RDY_OK;
}
static const struct BaseSequentialStreamVMT vmt = {
  NULL, NULL, putt, NULL
};

void shiftOutByte(uint8_t data)
{
   int i;
   for (i = 0; i < 8; i++)
   {
      palClearPad(GPIOA, GPIOA_LCD_CLK);
      if (data & (0x80 >> i))
      {
         palSetPad(GPIOA, GPIOA_LCD_DATA);
      }
      else
      {
         palClearPad(GPIOA, GPIOA_LCD_DATA);
      }
      palSetPad(GPIOA, GPIOA_LCD_CLK);
   }
}

// Writes a byte of data to the LCD.
void writeLcd(uint8_t data)
{
	//set the LCD's E (Enable) line high, so it can fall later
   palSetPad(GPIOD, GPIOD_LCD_E);
	//write the data to the bus
   shiftOutByte(data);
	//set the LCD's E (Enable) line low to latch in the data
   palClearPad(GPIOD, GPIOD_LCD_E);
}

// Writes a command byte to the LCD.
void writeControl(const uint8_t data)
{
	//set RS (Register Select) line low to select command register
   palClearPad(GPIOD, GPIOD_LCD_RS);
	writeLcd(data);
	//wait for the instruction to be executed
	aiDelayUs(100);
}

// Clears all characters on the display and resets the cursor to the home position.
void aiLCDClear(void)
{
	writeControl(0x01);
	aiDelayUs(3300);
}

// Shows the characters on the screen, if they were hidden with lcdOff().
void lcdOn(void)
{
	writeControl(0x0C);
}

// Hides the characters on the screen. Can be unhidden again with lcdOn().
void lcdOff(void)
{
	writeControl(0x08);
}

/* Initializes the LCD as described in the HD44780 datasheet.
   Normally called only by the initialize() function in utility.c.
 */
void aiLCDInit(void)
{
   LCD.vmt = &vmt;
  
	//set LCD E (Enable) line low inititally, so it can rise later
   palClearPad(GPIOD, GPIOD_LCD_E);

	//wait 15ms after power on
	aiDelayMs(15);

	//Issue 'Function Set' commands to initialize LCD for 8-bit interface mode
	writeControl(0x38);
	aiDelayUs(4900); //+100us in writeControl = 5000us or 5ms total
	writeControl(0x38);
	aiDelayUs(50); //+100us in writeControl = 150us total
	writeControl(0x38);

	//Function Set command to specify 2 display lines and character font
	writeControl(0x38);

	//Display off
	lcdOff();

	//Clear display
	aiLCDClear();

	//Set entry mode
	writeControl(0x06);

	//Display on
	lcdOn();
}

/* Prints a single character specified by its ASCII code to the display.
    Most LCDs can also print some special characters, such as those in LCDSpecialChars.h.
 */
void aiLCDPrintChar(const char data)
{
   if (data == '\n')
   {
      aiLCDBottomLine();
      return;
   }
   
	//set RS (Register Select) line high to select data register
   palSetPad(GPIOD, GPIOD_LCD_RS);
	writeLcd(data);
	aiDelayUs(50);
}

// Moves the LCD cursor to the beginning of the first line of the display (row 0, col 0).
void aiLCDTopLine()
{
	writeControl(HOME);
}

// Moves the LCD cursor to the beginning of the second line of the display (row 1, col 0).
void aiLCDBottomLine()
{
	writeControl(SECOND_LINE);
}