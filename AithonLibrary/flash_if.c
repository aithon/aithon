/**
  ******************************************************************************
  * @file    STM32F4xx_IAP/src/flash_if.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    10-October-2011
  * @brief   This file provides all the memory related operation functions.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/** @addtogroup STM32F4xx_IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"

const uint32_t FLASH_SECTOR_ADDR[NUM_FLASH_SECTORS+1] = {
	0x08000000, // Base address of Sector 0, 16 Kbyte */
	0x08004000, // Base address of Sector 1, 16 Kbyte */
	0x08008000, // Base address of Sector 2, 16 Kbyte */
	0x0800C000, // Base address of Sector 3, 16 Kbyte */
	0x08010000, // Base address of Sector 4, 64 Kbyte */
	0x08020000, // Base address of Sector 5, 128 Kbyte */
	0x08040000, // Base address of Sector 6, 128 Kbyte */
	0x08060000, // Base address of Sector 7, 128 Kbyte */
	// 512KB
	0x08080000, // Base address of Sector 8, 128 Kbyte */
	0x080A0000, // Base address of Sector 9, 128 Kbyte */
	0x080C0000, // Base address of Sector 10, 128 Kbyte */
	0x080E0000, // Base address of Sector 11, 128 Kbyte */
	// 1MB
	0x08100000, // Base address of Sector 13, 16 Kbyte */
	0x08104000, // Base address of Sector 13, 16 Kbyte */
	0x08108000, // Base address of Sector 14, 16 Kbyte */
	0x0810C000, // Base address of Sector 15, 16 Kbyte */
	0x08110000, // Base address of Sector 16, 64 Kbyte */
	0x08120000, // Base address of Sector 17, 128 Kbyte */
	0x08140000, // Base address of Sector 18, 128 Kbyte */
	0x08160000, // Base address of Sector 19, 128 Kbyte */
	// 1.5MB
	0x08180000, // Base address of Sector 20, 128 Kbyte */
	0x081A0000, // Base address of Sector 21, 128 Kbyte */
	0x081C0000, // Base address of Sector 22, 128 Kbyte */
	0x081E0000, // Base address of Sector 23, 128 Kbyte */
	// 2MB
	0x08200000  // NOT A REAL FLASH SECTOR
};

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Unlocks Flash for write access
  * @param  None
  * @retval None
  */
void FLASH_If_Init(void)
{ 
  FLASH_Unlock(); 

  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
}

// Async erase wrapper function
static int _nextEraseSector = -1;
static int _currentEraseSector = 0;
FLASH_EraseResult FLASH_If_Erase_Start(uint16_t startSector)
{
   _nextEraseSector = startSector;
   _currentEraseSector = 0;
   return FLASH_ERASE_IN_PROGRESS;
}
FLASH_EraseResult FLASH_If_Erase_Status(uint16_t endSector)
{
   // Check if we haven't started yet
   if (_nextEraseSector == -1)
      return FLASH_ERASE_ERROR;
   
   // CHeck if we need to move to the next sector
   if (_nextEraseSector != _currentEraseSector)
   {
      _currentEraseSector = _nextEraseSector;
      if (FLASH_EraseSectorStart(_currentEraseSector, VoltageRange_3) != FLASH_COMPLETE)
         return FLASH_ERASE_ERROR;
      else
         return FLASH_ERASE_IN_PROGRESS;
   }
   
   FLASH_Status status = FLASH_EraseSectorGetStatus();
   if (status == FLASH_BUSY)
   {
      return FLASH_ERASE_IN_PROGRESS;
   }
   else if (status == FLASH_COMPLETE)
   {
      _nextEraseSector += FLASH_SECTOR_DIFF;
      if (_nextEraseSector > endSector)
         return FLASH_ERASE_COMPLETE;
      else
         return FLASH_ERASE_IN_PROGRESS;
   }
   else
   {
      return FLASH_ERASE_COMPLETE;
   }
}

/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  FlashAddress: start address for writing data buffer
  * @param  Data: pointer on data buffer
  * @param  DataLength: length of data buffer (unit is 32-bit word)   
  * @retval 0: Data successfully written to Flash memory
  *         1: Error occurred while writing data in Flash memory
  *         2: Written Data in flash memory is different from expected one
  */
uint32_t FLASH_If_Write(__IO uint32_t* FlashAddress, uint32_t* Data ,uint32_t DataLength)
{
	uint32_t i = 0;

	for (i = 0; (i < DataLength) && (*FlashAddress < (FLASH_SECTOR_ADDR[APPLICATION_LAST_SECTOR+1]-1)); i++)
	{
		/* Device voltage range supposed to be [2.7V to 3.6V], the operation will
			be done by word */
		if (FLASH_ProgramWord(*FlashAddress, *(uint32_t*)(Data+i)) == FLASH_COMPLETE)
		{
			/* Check the written value */
			if (*(uint32_t*)*FlashAddress != *(uint32_t*)(Data+i))
			{
				/* Flash content doesn't match SRAM content */
				return 2;
			}
			/* Increment FLASH destination address */
			*FlashAddress += 4;
		}
		else
		{
			/* Error occurred while writing data in Flash memory */
			return 1;
		}
	}

	return 0;
}

int FLASH_Addr_To_Index(uint32_t addr)
{
   int i;
   for (i = 0; i < NUM_FLASH_SECTORS; i++)
   {
      if (addr >= FLASH_SECTOR_ADDR[i] && addr < FLASH_SECTOR_ADDR[i+1])
      {
         return i;
      }
   }
   
   return -1;
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
