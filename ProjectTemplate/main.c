#include "Aithon.h"

int main(void)
{
   // setup the board
   // must be called first
   aiInit();
   
   // print something to the LCD screen
   aiLCDPrintf("Aithon 1.0\nCP Robotics Club");
   
   // start with led 1 on
   aiLEDOn(0);
   aiLEDOff(1);
   
   while (1)
	{
      // main thread code here
      aiLEDToggle(0);
      aiLEDToggle(1);
      aiDelayS(1);
   }
}