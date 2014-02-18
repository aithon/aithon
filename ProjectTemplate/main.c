#include "Aithon.h"

int main(void)
{
   // print something to the LCD screen
   lcd_printf("Hello Aithon!");
   
   // start with led 0 on
   led_on(0);
   
   // main should never return, so enter infinite loop
   SerialDriver *dev = (SerialDriver *)&SDU1;
   while (1)
   {
      while (!sdGetWouldBlock(dev))
         sdPut(dev, sdGet(dev));
      delayMs(1);
   }
}
