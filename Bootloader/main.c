#include "main.h"

#ifndef DATE
#define DATE "unknown"
#endif
#define debugPrintf(fmt, ...) chprintf((BaseSequentialStream *)&SD2, fmt, ##__VA_ARGS__)

void debugPrintCmd(int cmdByte)
{
   switch (cmdByte)
   {
   case SYNC:
      debugPrintf("CMD: SYNC\r\n");
      break;
   case SET_START_ADDR:
      debugPrintf("CMD: SET_START_ADDR\r\n");
      break;
   case SET_PROG_LEN:
      debugPrintf("CMD: SET_PROG_LEN\r\n");
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
      debugPrintf("CMD: UNKNOWN (0x%x|%d", cmdByte, cmdByte);
      if (cmdByte < ' ')
         debugPrintf(")\r\n", cmdByte);
      else
         debugPrintf("|%c)\r\n", cmdByte);
   }
}

void sendResponse(uint8_t command, uint8_t response)
{
   sdPut(_interface, response|command);
}

int getByte(void)
{
   return sdGetTimeout(_interface, DEFAULT_TIMEOUT);
}

bool_t getBytes(uint8_t *data, int numBytes)
{
   int temp, i;
   for (i = 0; i < numBytes; i++)
   {
      if ((temp = getByte()) == Q_TIMEOUT)
         return FALSE;
      data[i] = (uint8_t)temp;
   }
   
   return TRUE;
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

extern int __ram_start__, __ram_end__;
bool_t isValidProgramStart(uint32_t addr)
{
   uint32_t stackEnd = (*(__IO uint32_t*)(addr));
   uint32_t resetVector = (*(__IO uint32_t*)(addr + 4));
   if (resetVector == 0xFFFFFFFF)
      return FALSE;
   if (resetVector < addr)
      return FALSE;
   if (stackEnd < (uint32_t)&__ram_start__ || stackEnd > (uint32_t)&__ram_end__)
      return FALSE;
      
   return TRUE;
}

void startProgram(uint32_t startAddr)
{
   if (!isValidProgramStart(startAddr))
   {
      lcd_clear();
      lcd_printf("No valid\nprogram found!");
      while(1);
   }
   /* Jump to user application */
   funcPtr userAppStart = (funcPtr) (*(__IO uint32_t*) (startAddr + 4));
   /* Initialize user application's Stack Pointer */
   __set_MSP(*(__IO uint32_t*) startAddr);
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
   int cmdByte;
   FLASH_EraseResult result;
   uint32_t addr, temp;
   int startSector = -1, endSector = -1;
   uint8_t tempByte = 0;

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
      case SET_START_ADDR:
         if (!getBytes((uint8_t *)&temp, 4))
            sendResponse(SET_START_ADDR, NACK);
         else
         {
            startSector = FLASH_Addr_To_Index(temp);
            if (startSector == -1)
               sendResponse(SET_START_ADDR, NACK);
            else
               sendResponse(SET_START_ADDR, ACK);
         }
         break;
      case SET_PROG_LEN:
         if (!getBytes((uint8_t *)&temp, 4))
            sendResponse(SET_PROG_LEN, NACK);
         else
         {
            uint32_t endAddr = FLASH_SECTOR_ADDR[startSector] + (temp * PACKET_LEN) - 1;
            endSector = FLASH_Addr_To_Index(endAddr);
            sendResponse(SET_PROG_LEN, ACK);
         }
         break;
      case ERASE_FLASH_START:
         if (startSector != -1 && FLASH_If_Erase_Start(FLASH_SECTORS[startSector]) == FLASH_ERASE_IN_PROGRESS)
            sendResponse(ERASE_FLASH_START, ACK);
         else
            sendResponse(ERASE_FLASH_START, NACK);
         break;
      case ERASE_FLASH_STATUS:
         result = FLASH_If_Erase_Status(FLASH_SECTORS[endSector]);
         if (result == FLASH_ERASE_COMPLETE)
            sendResponse(ERASE_FLASH_STATUS, ACK);
         else if (result == FLASH_ERASE_IN_PROGRESS)
            sendResponse(ERASE_FLASH_STATUS, BUSY);
         else
            sendResponse(ERASE_FLASH_STATUS, NACK);
         break;
      case SET_ADDR:
         // Read in the address
         if (!getBytes((uint8_t *)&addr, 4))
            sendResponse(SET_ADDR, NACK);
         else
         {
            sendResponse(SET_ADDR, ACK);
            // We'll get relative addresses, so add the start address.
            addr += FLASH_SECTOR_ADDR[startSector];
            debugPrintf("Set ADDR to %x\r\n", addr);
         }
         break;
      case CHECK_ADDR:
         // Get the checksum
         if (!getBytes((uint8_t *)&tempByte, 1))
            sendResponse(CHECK_ADDR, NACK);
         else
         {
            // Subtract the start address before calculating the checksum
            addr -= FLASH_SECTOR_ADDR[startSector];
            if (tempByte == calcChecksum((uint8_t *)&addr, 4))
               sendResponse(CHECK_ADDR, ACK);
            else
               sendResponse(CHECK_ADDR, NACK);
            addr += FLASH_SECTOR_ADDR[startSector];
         }
         break;
      case FILL_BUFFER:
         if (!getBytes(_buffer, PACKET_LEN))
            sendResponse(FILL_BUFFER, NACK);
         else
            sendResponse(FILL_BUFFER, ACK);
         break;
      case CHECK_BUFFER:
         // Get the checksum
         if (!getBytes((uint8_t *)&tempByte, 1) || tempByte != calcChecksum(_buffer, PACKET_LEN))
            sendResponse(CHECK_BUFFER, NACK);
         else
            sendResponse(CHECK_BUFFER, ACK);
         break;
      case COMMIT_BUFFER:
         debugPrintf("Writing at %x\r\n", addr);
         if (FLASH_If_Write((__IO uint32_t *)&addr, (uint32_t *)_buffer, PACKET_LEN/4))
            sendResponse(COMMIT_BUFFER, NACK);
         else
            sendResponse(COMMIT_BUFFER, ACK);
         break;
      case START_PROGRAM:
         sendResponse(START_PROGRAM, ACK);
         flushInterface();
         delayMs(100);
         startProgram(USER_START_ADDR);
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
   uint16_t bootOptions;
   _ee_getReserved(_AI_EE_RES_BOOT_OPTIONS, &bootOptions);
   
   if (button_get(0) && button_get(1))
      // Both buttons are pressed so the user
      // wants to run the bootloader.
      isUserRun = TRUE;
   else if (button_get(1))
      startProgram(DEMO_START_ADDR);
   else if ((bootOptions >> 12) != BOOT_MAGIC)
   {
      if (isValidProgramStart(USER_START_ADDR))
         startProgram(USER_START_ADDR);
      else
         // If there's no valid program, run the
         // bootloader in user-run mode.
         isUserRun = TRUE;
   }
   _ee_putReserved(_AI_EE_RES_BOOT_OPTIONS, (bootOptions & 0x0FFF));

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
            lcd_printf("Aithon Board  %2d\n%s", (BOOT_TIMEOUT-i)/1000, DATE);
         }

         // start the user program if button 0 is pressed
         if (button_get(0) && i > 1000 && isValidProgramStart(DEMO_START_ADDR)) 
            startProgram(USER_START_ADDR);

         // start the demo program if button 1 is pressed
         if (button_get(1) && i > 1000 && isValidProgramStart(DEMO_START_ADDR)) 
            startProgram(DEMO_START_ADDR);
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
   startProgram(USER_START_ADDR);
   return 0;
}
