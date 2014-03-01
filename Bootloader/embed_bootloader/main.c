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
   for (i = FLASH_SECTORS[BOOTLOADER_FIRST_SECTOR]; i <= FLASH_SECTORS[BOOTLOADER_LAST_SECTOR]; i += FLASH_SECTOR_DIFF)
   {
      /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       *                 be done by word */
      if (FLASH_EraseSector(i, VoltageRange_3) != FLASH_COMPLETE)
      {
         /* Error occurred while page erase */
         lcd_clear();
         lcd_printf ("Error during\npage erase #%d", i);
         while(1) {
            chThdSleepMilliseconds(1);
         }
      }
   }

   //write the bootloader to the flash
   addr = FLASH_SECTOR_ADDR[BOOTLOADER_FIRST_SECTOR];
   stat = FLASH_If_Write((__IO uint32_t *)&addr, (uint32_t *)bootloader, lines);
   lcd_clear();
   if (stat == 2) {
      lcd_printf ("Verification\nError");
   } else if (stat == 1) {
      lcd_printf ("Error writing\nflash");
   } else {
      lcd_printf ("Success");
   }

   while(1) {
      chThdSleepMilliseconds(1);
   }
}

int main(void)
{
   int i;
   unsigned int data;

   //check that the bootloader has not already been flashed
   for (i=0;i<lines;i++) {
      data = *(__IO uint32_t*) (FLASH_SECTOR_ADDR[BOOTLOADER_FIRST_SECTOR] + i*4); 
      if (bootloader[i] != data) {
         break;
      }
   }

   if (i == lines) {
      lcd_clear();
      lcd_printf("Bootloader\nalready updated");
      while(1) {
         chThdSleepMilliseconds(1);
      }
   }

   lcd_clear();
   lcd_printf("Press BTN0 for\n");
   lcd_printf("new bootloader");
  
   //wait for button press
   while(button_get(0) == FALSE) {
      chThdSleepMilliseconds(1);
   }

   updateBootloader();
   return 0;
}
