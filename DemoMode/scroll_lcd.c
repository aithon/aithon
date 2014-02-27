#include "Aithon.h"

static WORKING_AREA(scrollingThreadWorkingArea, 128);
Mutex _scrollMtx;
int _scrollRow = 0, _scrollCol = 0;
int _scrollDelay = 300; //scrolling delay in ms
int _scrollWidth = 16;  //width of scrolling area
int _scrollMsgLen = 0;  //length of message to scroll (not counting terminating null)
int _scrollStatus = 1;  //1=scrolling, 2=paused, 0=exit scrolling thread
int _scrollStart = 1;   //1=first time message is scrolled (give a slight pause) 
char _scrollBuf[100];

//LCD scrolling thread
static msg_t scrollingThread(void *arg) 
{
   (void)arg;
   int cur_start;
   int i;
   int j=3;
   int times;
   
   cur_start = 0;

   while (_scrollStatus != 0) 
   {
      //if a new message was just set, reset _scrollStart to give an initial pause
      if (_scrollStart == 1) 
      {
         _scrollStart++;
         cur_start = 0;
         j = 3;
      }

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
         if (_scrollStart != 0 && j > 0) 
         {
            j--;
         } else {
            _scrollStart=0;
            cur_start++;
            if (cur_start == _scrollMsgLen) 
               cur_start = 0;
         }
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

   //error check to prevent exceeding 16 characters
   if ((col + window) > 16) 
   {
      window = 16-col;
   }

   _scrollWidth = window;
   _scrollStart = 1;
}
