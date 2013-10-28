#include "main.h"

void sendByte(uint8_t byte)
{
   sdPut(_interface, byte);
}

int getByte(void)
{
   return sdGetTimeout(_interface, DEFAULT_TIMEOUT);
}

void startProgram(void)
{
	/* Jump to user application */
	funcPtr userAppStart = (funcPtr) (*(__IO uint32_t*) (APPLICATION_ADDRESS + 4));
	/* Initialize user application's Stack Pointer */
	__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
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

   aiLCDBottomLine();
   aiLCDPrintf("Programming...");
   
   while(1)
   {
      cmdByte = getByte();
      switch (cmdByte)
      {
      case SYNC:
         // initial sync
         sendByte(ACK);
         break;
      case ERASE_FLASH:
         // global flash erase
         sendByte(ACK);
         sendByte(!FLASH_If_Erase()?ACK:NACK);
         break;
      case SET_ADDR:
         addr = 0;
         for (i = 0; i < 4; i++)
         {
            if ((temp = getByte()) == Q_TIMEOUT)
            {
               sendByte(ABORT);
               return;
            }
            addr |= (((uint8_t) temp) & 0xFF) << (i * 8);
         }
         sendByte(ACK);
         sendByte(calcChecksum((uint8_t *)&addr, 4));
         break;
      case FILL_BUFFER:
         for (i = 0; i < PACKET_LEN; i++)
         {
            if ((temp = getByte()) == Q_TIMEOUT)
            {
               sendByte(ABORT);
               return;
            }
            _buffer[i] = (uint8_t) (temp & 0xFF);
         }
         sendByte(ACK);
         sendByte(calcChecksum(_buffer, PACKET_LEN));
         break;
      case COMMIT_BUFFER:
         if (FLASH_If_Write((__IO uint32_t *)&addr, (uint32_t *)_buffer, PACKET_LEN/4))
            sendByte(NACK);
         else
            sendByte(ACK);
         break;
      case START_PROGRAM:
         sendByte(ACK);
         aiLCDBottomLine();
         aiLCDPrintf("ABOUT TO START!!");
         aiDelayS(2);
         startProgram();
         // ...should never get here
         return;
      case Q_TIMEOUT:
         break;
      default:
         aiLCDBottomLine();
         aiLCDPrintf("DONE %d", cmdByte);
         aiDelayS(2);
         sendByte(NACK);
         return;
      }
   }
}

int main(void)
{
   halInit();
   chSysInit();
   aiInit();
   
   aiLCDPrintf("Aithon Board");
   // /* Unlock the Flash Program Erase controller */
   FLASH_If_Init();
   
   sdStart(&SD1, NULL);
   sdStart(&SD2, NULL);
   
   int i, j;
   for (i = 0; i < BOOT_TIMEOUT; i++)
   {
      // update the countdown
      if (i % 1000 == 0)
      {
         aiLCDBottomLine();
         aiLCDPrintf("%d", (BOOT_TIMEOUT-i)/1000);
      }
      // check all the interfaces for a SYNC
      for (j = 0; j < NUM_INTERFACES; j++)
      {
         if (sdGetTimeout(_interfaces[j], TIME_IMMEDIATE) == SYNC)
         {
            _interface = _interfaces[j];
            updateProgram();
            // if we get here, we didn't program successfully
            aiLCDBottomLine();
            aiLCDPrintf("Prog Failed   :(");
            while(1);
         }
      }
      chThdSleepMilliseconds(1);
   }
	startProgram();
   return 0;
}