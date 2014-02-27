#include "main.h"

#ifndef DATE
#define DATE "unknown"
#endif
#define debugPrintf(fmt, ...) chprintf((BaseSequentialStream *)&SD2, fmt, ##__VA_ARGS__)

extern Mutex _scrollMtx;

void debugPrintCmd(int cmdByte)
{
   switch (cmdByte)
   {
   case SYNC:
      debugPrintf("CMD: SYNC\r\n");
      break;
   case ERASE_FLASH_START:
      debugPrintf("CMD: ERASE_FLASH_START\r\n");
      break;
   case ERASE_FLASH_STATUS:
      debugPrintf("CMD: ERASE_FLASH_STATUS\r\n");
      break;
   case SET_ADDR:
      debugPrintf("CMD: SET_ADDR\r\n");
      break;
   case CHECK_ADDR:
      debugPrintf("CMD: CHECK_ADDR\r\n");
      break;
   case FILL_BUFFER:
      debugPrintf("CMD: FILL_BUFFER\r\n");
      break;
   case CHECK_BUFFER:
      debugPrintf("CMD: CHECK_BUFFER\r\n");
      break;
   case COMMIT_BUFFER:
      debugPrintf("CMD: COMMIT_BUFFER\r\n");
      break;
   case START_PROGRAM:
      debugPrintf("CMD: START_PROGRAM\r\n");
      break;
   case Q_TIMEOUT:
      debugPrintf("CMD: Q_TIMEOUT\r\n");
      break;
   default:
      debugPrintf("CMD: UNKNOWN (0x%x|%d|%c)\r\n", cmdByte, cmdByte, cmdByte);
      break;
   }
}

void sendResponse(uint8_t command, uint8_t response)
{
   sdPut(_interface, response|command);
}

int getByte(void)
{
   return sdGetTimeout(_interface, DEFAULT_TIMEOUT);
   //uint8_t cmd = 0;
   //sdRead(_interface,&cmd,1);
   //return cmd;
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

void startDemoProgram(void)
{
   /* Jump to user application */
   funcPtr userAppStart = (funcPtr) (*(__IO uint32_t*) (DEMO_START_ADDRESS + 4));
   /* Initialize user application's Stack Pointer */
   __set_MSP(*(__IO uint32_t*) DEMO_START_ADDRESS);
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
   int clear = 0;
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
      led_toggle(0);
      cmdByte = getByte();
      debugPrintCmd(cmdByte);
      switch (cmdByte)
      {
      case SYNC:
         // sync
         flushInterface();
         sendResponse(SYNC, ACK);
         break;
      case ERASE_FLASH_START:
         if (FLASH_If_Erase_Start() == FLASH_ERASE_IN_PROGRESS)
            sendResponse(ERASE_FLASH_START, ACK);
         else
            sendResponse(ERASE_FLASH_START, NACK);
         break;
      case ERASE_FLASH_STATUS:
         result = FLASH_If_Erase_Status(endSector);
         if (result == FLASH_ERASE_COMPLETE)
            sendResponse(ERASE_FLASH_STATUS, ACK);
         else if (result == FLASH_ERASE_IN_PROGRESS)
            sendResponse(ERASE_FLASH_STATUS, BUSY);
         else
            sendResponse(ERASE_FLASH_STATUS, NACK);
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
            sendResponse(SET_ADDR, NACK);
         else
         {
            sendResponse(SET_ADDR, ACK);
            // We'll get relative addresses, so add the start address.
            addr += APPLICATION_START_ADDRESS;
         }
         break;
      case CHECK_ADDR:
         // Get the checksum
         temp = getByte();
         if (temp == Q_TIMEOUT)
            sendResponse(CHECK_ADDR, NACK);
         else
         {
            // Subtract the start address before calculating the checksum
            addr -= APPLICATION_START_ADDRESS;
            if (temp == calcChecksum((uint8_t *)&addr, 4))
               sendResponse(CHECK_ADDR, ACK);
            else
               sendResponse(CHECK_ADDR, NACK);
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
            sendResponse(FILL_BUFFER, NACK);
         else
            sendResponse(FILL_BUFFER, ACK);
         break;
      case CHECK_BUFFER:
         // Get the checksum
         temp = getByte();
         if (temp != Q_TIMEOUT && temp == calcChecksum(_buffer, PACKET_LEN))
            sendResponse(CHECK_BUFFER, ACK);
         else
            sendResponse(CHECK_BUFFER, NACK);
         break;
      case COMMIT_BUFFER:
         maxAddr = addr + PACKET_LEN - 1;
         if (FLASH_If_Write((__IO uint32_t *)&addr, (uint32_t *)_buffer, PACKET_LEN/4))
            sendResponse(COMMIT_BUFFER, NACK);
         else
            sendResponse(COMMIT_BUFFER, ACK);
         break;
      case START_PROGRAM:
         sendResponse(START_PROGRAM, ACK);
         flushInterface();
         _ee_putReserved(_AI_EE_RES_ADDR_MAX_SECTOR, FLASH_Addr_To_Sector(maxAddr));
         delayS(1);
         startProgram();
         // ...should never get here
         return;
      case Q_TIMEOUT:
      default:
         if (clear == 0) {
           lcd_clear();
           clear = 1;
         }
         lcd_printf ("0%x ", cmdByte); 
         break;
      }
   }
}

int main(void)
{
   bool_t isUserRun = FALSE;
   bool_t displayCountdown = TRUE;
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

   lcd_clear();

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
            lcd_cursor(0,0);
            lcd_printf("Aithon Board  %2d ", (BOOT_TIMEOUT-i)/1000);
            displayCountdown = TRUE;
         }

         // show the bootloader build date if button 0 pressed
         if (button_get(0) && displayCountdown) 
         {
            if (i > (.03 * BOOT_TIMEOUT)) 
            {
               startDemoProgram();
            }
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
