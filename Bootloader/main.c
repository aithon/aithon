#include "main.h"

void sendByte(uint8_t byte)
{
   sdPut(_interface, byte);
}

int getByte(void)
{
   return sdGetTimeout(_interface, DEFAULT_TIMEOUT);
}

void flushInterface(void)
{
   // empty output buffer
   while (sdPutWouldBlock(_interface));

   // very small sleep to reduce chance of race conditions
   chThdSleepMilliseconds(1);

   // empty input buffer
   while (!sdGetWouldBlock(_interface))
      sdGet(_interface);
}

void startProgram(void)
{
   /* Jump to user application */
   funcPtr userAppStart = (funcPtr) (*(__IO uint32_t*) (APPLICATION_START_ADDRESS + 4));
   /* Initialize user application's Stack Pointer */
   __set_MSP(*(__IO uint32_t*) APPLICATION_START_ADDRESS);
   userAppStart();
}

uint8_t calcChecksum(uint8_t *bytes, int len)
{
   int i;
   uint8_t checksum = 0;
   for (i = 0; i < len; i++)
   {
      checksum ^= bytes[i];
   }
   return checksum;
}


void updateProgram(void)
{
   int cmdByte, i, temp;
   FLASH_EraseResult result;
   uint32_t addr, maxAddr = 0;
   uint16_t endSector = 0xFFFF;
   _ee_getReserved(_AI_EE_RES_ADDR_MAX_SECTOR, &endSector);
   if (endSector > APPLICATION_END_SECTOR || !IS_FLASH_SECTOR(endSector))
      endSector = APPLICATION_END_SECTOR;

   lcd_clear();
   lcd_printf("Aithon Board\nProgramming...");

   // Unlock the Flash Program Erase controller
   FLASH_If_Init();

   while (TRUE)
   {
      cmdByte = getByte();
      switch (cmdByte)
      {
      case SYNC:
         // sync
         flushInterface();
         sendByte(SYNC);
         break;
      case ERASE_FLASH_START:
         sendByte((FLASH_If_Erase_Start() == FLASH_ERASE_IN_PROGRESS)?ACK:NACK);
         break;
      case ERASE_FLASH_STATUS:
         result = FLASH_If_Erase_Status(endSector);
         if (result == FLASH_ERASE_COMPLETE)
            sendByte(ACK);
         else if (result == FLASH_ERASE_IN_PROGRESS)
            sendByte(BUSY);
         else
            sendByte(NACK);
         break;
      case SET_ADDR:
         // Read in the address, MSB first.
         addr = 0;
         for (i = 0; i < 4; i++)
         {
            if ((temp = getByte()) == Q_TIMEOUT)
               break;
            addr |= (((uint8_t) temp) & 0xFF) << (i * 8);
         }

         // Check for errors.
         if (temp == Q_TIMEOUT)
            sendByte(NACK);
         else
         {
            sendByte(ACK);
            sendByte(calcChecksum((uint8_t *)&addr, 4));
            // We'll get relative addresses, so add the start address.
            addr += APPLICATION_START_ADDRESS;
         }
         break;
      case FILL_BUFFER:
         for (i = 0; i < PACKET_LEN; i++)
         {
            if ((temp = getByte()) == Q_TIMEOUT)
               break;
            _buffer[i] = (uint8_t) (temp & 0xFF);
         }
         if (temp == Q_TIMEOUT)
            sendByte(NACK);
         else
         {
            sendByte(ACK);
            sendByte(calcChecksum(_buffer, PACKET_LEN));
         }
         break;
      case COMMIT_BUFFER:
         maxAddr = addr + PACKET_LEN - 1;
         if (FLASH_If_Write((__IO uint32_t *)&addr, (uint32_t *)_buffer, PACKET_LEN/4))
            sendByte(NACK);
         else
            sendByte(ACK);
         break;
      case START_PROGRAM:
         sendByte(ACK);
         flushInterface();
         _ee_putReserved(_AI_EE_RES_ADDR_MAX_SECTOR, FLASH_Addr_To_Sector(maxAddr));
         startProgram();
         // ...should never get here
         return;
      case Q_TIMEOUT:
      default:
         break;
      }
   }
}

int main(void)
{
   bool_t isUserRun = FALSE;
   uint16_t bootByte;
   _ee_getReserved(_AI_EE_RES_ADDR_BOOT, &bootByte);
   if (button_get(0) && button_get(1))
   {
      // Both buttons are pressed so the user
      // wants to run the bootloader.
      isUserRun = TRUE;
   }
   else if (bootByte != _AI_EE_RES_VAL_BOOT_RUN)
   {
      startProgram();
   }
   _ee_putReserved(_AI_EE_RES_ADDR_BOOT, _AI_EE_RES_VAL_DEFAULT);

   led_on(0);
   led_on(1);

   int i, j;
   for (i = 0; i < BOOT_TIMEOUT; i++)
   {
      if (isUserRun)
      {
         // flash LED1
         if (i % 100 == 0)
            led_toggle(1);
         
         // update the countdown
         if (i % 1000 == 0)
         {
            lcd_cursor(0, 0);
            lcd_printf("Aithon Board\n%d ", (BOOT_TIMEOUT-i)/1000);
         }
      }

      // check all the interfaces for a SYNC
      for (j = 0; j < NUM_INTERFACES; j++)
      {
         if (sdGetTimeout(_interfaces[j], TIME_IMMEDIATE) == SYNC)
         {
            _interface = _interfaces[j];
            updateProgram();
            // We should never get here...
            while(1);
         }
      }
      chThdSleepMilliseconds(1);
   }
   startProgram();
   return 0;
}
