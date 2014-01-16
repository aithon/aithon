#include "Tester.h"

int test_digital_pin(int pin)
{
   // Set as INPUT_PULLUP and make sure it reads as a DIGITAL_HIGH
   delayMs(1);
   digital_mode(pin, INPUT_PULLUP);
   delayMs(1);
   if (digital_get(pin) != DIGITAL_HIGH)
      return 1;
      
   // Set as INPUT_PULLDOWN and make sure it reads as a DIGITAL_LOW
   delayMs(1);
   digital_mode(pin, INPUT_PULLDOWN);
   delayMs(1);
   if (digital_get(pin) != DIGITAL_LOW)
      return 2;
      
   // Test outputting LOW then changing to input
   // and checking that the pin reads as a LOW
   digital_mode(pin, INPUT);
   delayMs(50);
   digital_mode(pin, OUTPUT);
   digital_set(pin, DIGITAL_HIGH);
   delayMs(1);
   digital_mode(pin, INPUT);
   if (digital_get(pin) != DIGITAL_HIGH)
      return 3;
      
   // Test outputting HIGH then changing to input
   // and checking that the pin reads as a HIGH
   digital_mode(pin, INPUT);
   delayMs(50);
   digital_mode(pin, OUTPUT);
   digital_set(pin, DIGITAL_LOW);
   delayMs(1);
   digital_mode(pin, INPUT);
   if (digital_get(pin) != DIGITAL_LOW)
      return 4;
      
   return 0;
}

int test_digital_pins()
{
   int failed = 0;
   int i;
   for (i = 0; i < NUM_DIGITAL_PINS; i++)
   {
      if (test_digital_pin(i))
         failed++;
      delayMs(100);
   }
   lcd_cursor(0, 1);
   return failed;
}