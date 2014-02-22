#include "Aithon.h"

static WORKING_AREA(scrollingThreadWorkingArea, 128);
int scroll_row=0, scroll_col=0;
int scroll_delay=300; //scrolling delay in ms
int scroll_window=16; //width of scrolling area
int message_length=0; //length of message to scroll (not counting terminating null)
int scroll_status=1;  //1=scrolling, 2=paused, 0=exit scrolling thread
char scroll_buf[100];

//LCD scrolling thread
//call this after calling scrollMessage
static msg_t scrollingThread(void *arg) 
{
   (void)arg;
   int cur_start;
   int i;
   int times;
   
   cur_start = 0;

   while (scroll_status != 0) 
   {
      if (scroll_status == 1) 
      {
         //set cursor position
         lcd_cursor(scroll_col,scroll_row);

         i=cur_start;
         times=0;

         //print the message
         while (times < scroll_window)
         {
            lcd_printChar(scroll_buf[i]);
            i++;
            times++;

            if (i == message_length)
               i = i % message_length;
         }

         //adjust the scroll position
         cur_start++;
         if (cur_start == message_length) 
            cur_start = 0;
      }

      chThdSleepMilliseconds(scroll_delay);
   }

   return 0;
}
 
void scroll_init(void) 
{
   //start the scrolling thread
   (void)chThdCreateStatic(scrollingThreadWorkingArea, sizeof(scrollingThreadWorkingArea),
         NORMALPRIO, scrollingThread, NULL);

}

//set starting position of scroll message on the LCD
void scrollSetCursor(int row, int col) 
{
   scroll_row=row;
   scroll_col=col;
}

void scrollSetDelay(int delay) {
   scroll_delay=delay;
}

//enable scrolling
void scrollEnable(void) {
   scroll_status = 1;
}

//pause scrolling
void scrollPause(void) {
   scroll_status = 2;
}

//end scrolling thread
void scrollExit(void) {
   scroll_status = 0;
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
   int i=0;
   int j;

   //copy string
   while(m[i] != 0)
   {
      scroll_buf[i] = m[i];
      i++;
   }
   
   //add a few blank spaces at the end of the message
   for (j=0; j<3; j++) 
   {
      scroll_buf[i] = ' ';
      i++;
   }

   scroll_buf[i] = m[i]; //copy null
   message_length = i; //message length not counting the null
   scrollSetCursor(row,col);

   if ((col+window) > 16) 
   {
      window = 16-col;
   }

   scroll_window=window;
}

