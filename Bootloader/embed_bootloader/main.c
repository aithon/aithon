#include "main.h"
#include "embed_bootloader.h"

#define debugPrintf(fmt, ...) chprintf((BaseSequentialStream *)&SD2, fmt, ##__VA_ARGS__)

void updateBootloader(void)
{
   int stat;
   int i;
   uint32_t addr;

   lcd_clear();
   lcd_printf("Aithon Board\nProgramming...");

   // Unlock the Flash Program Erase controller
   FLASH_If_Init();

   //erase the bootloader flash sectors
   i = 0;
   for (i = FLASH_Sector_0; i <= FLASH_Sector_1; i += (FLASH_Sector_1-FLASH_Sector_0))
   {
      /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       *                 be done by word */
      if (FLASH_EraseSector(i, VoltageRange_3) != FLASH_COMPLETE)
      {
         /* Error occurred while page erase */
         lcd_clear();
         lcd_printf ("Error during\npage erase #%d", i);
         while(1) {}
      }
   }

   //write the bootloader to the flash
   addr = ADDR_FLASH_SECTOR_0;
   stat = FLASH_If_Write((__IO uint32_t *)&addr, (uint32_t *)bootloader, lines);
   lcd_clear();
   if (stat == 2) {
      lcd_printf ("Verification\nError");
   } else if (stat == 1) {
      lcd_printf ("Error writing\nflash");
   } else {
      lcd_printf ("Success");
   }

   while(1) {}
}

int main(void)
{
   lcd_clear();
   lcd_printf("Press BTN0 to flash\n");
   lcd_printf("new bootloader");
  
   //wait for button press
   while(button_get(0) == FALSE) {
      chThdSleepMilliseconds(1);
   }

   updateBootloader();
   return 0;
}
