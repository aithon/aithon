#include "Aithon.h"
#include "flash_if.h"

/* Private functions ---------------------------------------------------------*/

#define ACK          0x79
#define NACK         0x1F
#define PACKET_LEN   1024
#define BOOT_TIMEOUT 5000

typedef  void (*pFunction)(void);
pFunction Jump_To_Application;
uint32_t JumpAddress;

static SerialDriver *chan;

void sendByte(uint8_t byte)
{
   sdPut(chan, byte);
}

int getByte(void)
{
   return sdGetTimeout(chan, 100);
}

void startProgram(void)
{
	JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
	/* Jump to user application */
	Jump_To_Application = (pFunction) JumpAddress;
	/* Initialize user application's Stack Pointer */
	__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
	Jump_To_Application();
}

void downloadProgram(void)
{
   static uint8_t temp_data[PACKET_LEN];
   int i;
   // aiLCDBottomLine();
   // aiLCDPrintf("Programming...");
   aiLCDClear();
   while(1)
   {
      int cmdByte = getByte();
      switch (cmdByte)
      {
      case 0x7F:
         // initial sync
         sendByte(ACK);
         break;
      case 0x43:
         // global flash erase
         sendByte(ACK);
         sendByte(!FLASH_If_Erase()?ACK:NACK);
         break;
      case 0x31:
         sendByte(ACK);
         uint32_t addr = getByte();
         addr <<= 8;
         addr |= getByte();
         addr <<= 8;
         addr |= getByte();
         addr <<= 8;
         addr |= getByte();
         sendByte(ACK);
         
         uint8_t crc = 0xFF;
         uint16_t len = getByte();
         len <<= 8;
         len |= getByte();
         for (i = 0; i <= len; i++)
         {
            int c = getByte();
            if (c == Q_TIMEOUT)
            {
               sendByte(NACK);
               aiLCDPrintf("DATA TIMEOUT");
               aiDelayS(1);
               break;
            }
            crc ^= (uint8_t) c;
            temp_data[i] = c;
         }
         if (i > len)
         {
            for (; i < PACKET_LEN; i++)
            {
               temp_data[i] = 0xFF;
            }
            if (crc == getByte())
            {
               if (len & 0x03)
                  len += 4;
               len >>= 2;
               FLASH_If_Write(&addr, (uint32_t *)temp_data, len);
               sendByte(ACK);
            }
            else
            {
               sendByte(NACK);
            }
         }
         else
         {
            aiLCDPrintf("HERE");
            aiDelayS(1);
         }
         break;
      case 0x21:
         chThdSleepMilliseconds(100);
         sendByte(ACK);
         chThdSleepMilliseconds(100);
         startProgram();
         break;
      case 0x11:
         break;
      case Q_TIMEOUT:
         sendByte(NACK);
         break;
      default:
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
   aiUSBCDCInit();
   
   palSetPadMode(GPIOD, GPIOD_DIGITAL14, PAL_MODE_INPUT_PULLDOWN);
   
   int i;
   SerialDriver *channels[3] = {&SD1, &SD2, (SerialDriver *)&SDU1};
   
   for (i = 0; i < BOOT_TIMEOUT; i++)
   {
      if (i % 1000 == 0)
      {
         aiLCDBottomLine();
         aiLCDPrintf("%d", (BOOT_TIMEOUT-i)/1000);
      }
      int j;
      for (j = 0; j < 3; j++)
      {
         if (sdGetTimeout(channels[j], TIME_IMMEDIATE) == 0x11)
         {
            chan = channels[j];
            downloadProgram();
         }
      }
      if (palReadPad(GPIOD, GPIOD_DIGITAL14))
      {
         break;
      }
      chThdSleepMilliseconds(1);
   }
	startProgram();
   return 0;
}