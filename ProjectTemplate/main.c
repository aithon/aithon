#include "Aithon.h"

int main(void)
{
   int i;
   
   // print something to the LCD screen
   lcd_printf("Hello Aithon!");
   
   // start with led 0 on
   led_on(0);
   
   // main should never return, so enter infinite loop
   while (1)
	{
      // main thread code here
      led_toggle(0);
      led_toggle(1);
      
      for (i = 0; i < 8; i++)
      {
         while (!button_get(0))
         {
            lcd_clear();
            lcd_printf("A%d = %d", i, analog_get(i));
            
            
            delayMs(200);
            
         }
         delayMs(500);
      }
   }
}