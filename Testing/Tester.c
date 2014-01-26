#include "Tester.h"

#define NUM_TESTS (int)(sizeof tests / sizeof tests[0])

AithonTest tests[] = {
   {test_digital_pins, "Digital Pins"},
   // {test_sd_init, "SD Init"},
   {test_buzzer, "Buzzer"},
   // {test_motors, "Motors"},
};

int main(void)
{
   int i;
   for (i = 0; i < NUM_TESTS; i++)
   {
      // Print out a message saying the test is running
      lcd_printf(tests[i].name);
      lcd_cursor(0, 1);
      lcd_printf("Running...");
      
      // Run the test
      int numErrors = tests[i].start();
      
      // Print out a message saying the test passed / failed
      lcd_clear();
      lcd_printf(tests[i].name);
      lcd_cursor(0, 1);
      if (numErrors == 0)
         lcd_printf("Passed!");
      else
         lcd_printf("Failure: %d", numErrors);
         
      // Wait for button 0 to be pressed before moving on
      button_wait(0);
      lcd_clear();
   }
   
   lcd_printf("Done!");
   
   while (1)
   {
      delayS(100);
   }
}