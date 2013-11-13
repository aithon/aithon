#include "main.h"

void sendByte(uint8_t byte)
{
   sdPut(_interface, byte);
   // while (chOQGetFullI(&(_interface->oqueue)) > 0) chThdSleepMilliseconds(5);
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
   uint32_t addr;

   aiLCDTopLine();
   aiLCDPrintf("Aithon Board");
   aiLCDBottomLine();
   aiLCDPrintf("Programming...");

   // Unlock the Flash Program Erase controller
   FLASH_If_Init();

   while(1)
   {
      cmdByte = getByte();
      switch (cmdByte)
      {
      case SYNC:
         // sync
         flushInterface();
         sendByte(SYNC);
         break;
      case ERASE_FLASH:
         // global flash erase
         sendByte(!FLASH_If_Erase()?ACK:NACK);
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
         if (FLASH_If_Write((__IO uint32_t *)&addr, (uint32_t *)_buffer, PACKET_LEN/4))
            sendByte(NACK);
         else
            sendByte(ACK);
         break;
      case START_PROGRAM:
         sendByte(ACK);
         flushInterface();
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
   halInit();
   _aiBKSRAMInit();
   if (aiGetButton(0) && aiGetButton(1))
   {
      // Both buttons are pressed so the user
      // wants to run the bootloader.
      isUserRun = TRUE;
   }
   else if ((_AI_RESERVED_BYTE & 0x02) == 0)
   {
      startProgram();
   }
   _AI_RESERVED_BYTE &= ~0x02; // clear bootloader bit

   chSysInit();
   aiInit();
   aiLEDOn(0);
   aiLEDOn(1);


   int i, j;
   for (i = 0; i < BOOT_TIMEOUT; i++)
   {
      // update the countdown
      if (isUserRun && i % 1000 == 0)
      {
         aiLCDTopLine();
         aiLCDPrintf("Aithon Board");
         aiLCDBottomLine();
         aiLCDPrintf("%d ", (BOOT_TIMEOUT-i)/1000);
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
