#include "Aithon.h"
extern int _resetIndex;

int main(void)
{
   // print something to the LCD screen
   lcd_printf("Hello Aithon!\n");
   
   // start with led 0 on
   led_on(0);
   
   // main should never return, so enter infinite loop
   while (1)
   {
      lcd_clear();
      lcd_printf("%d\n", _resetIndex);
      lcd_printf("%d\n", chTimeNow());
      // main thread code here
      led_toggle(0);
      led_toggle(1);
      
      delayS(1);
   }
}
