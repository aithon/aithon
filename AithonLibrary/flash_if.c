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
FLASH_EraseResult FLASH_If_Erase_Start(uint16_t endSector)
{
   _nextEraseSector = APPLICATION_START_SECTOR;
   _currentEraseSector = 0;
   return FLASH_If_Erase_Status(endSector);
}
FLASH_EraseResult FLASH_If_Erase_Status(uint16_t endSector)
{
   if (_nextEraseSector == -1)
   {
      return FLASH_ERASE_ERROR;
   }
   
   if (_nextEraseSector != _currentEraseSector)
   {
      _currentEraseSector = _nextEraseSector;
      if (FLASH_EraseSectorStart(_currentEraseSector, VoltageRange_3) != FLASH_COMPLETE)
      {
         // Error occurred
         return FLASH_ERASE_COMPLETE;
      }
   }
   
   FLASH_Status status = FLASH_EraseSectorGetStatus();
   if (status == FLASH_BUSY)
   {
      return FLASH_ERASE_IN_PROGRESS;
   }
   else if (status == FLASH_COMPLETE)
   {
      _nextEraseSector += (FLASH_Sector_1-FLASH_Sector_0);
      if (_nextEraseSector > endSector)
         return FLASH_ERASE_COMPLETE;
      else
         return FLASH_If_Erase_Status(endSector);
   }
   else
   {
      return FLASH_ERASE_COMPLETE;
   }
}

/**
  * @brief  This function does an erase of all user flash area
  * @param  StartSector: start of user flash area
  * @retval 0: user flash area successfully erased
  *         1: in progress
  */
uint32_t FLASH_If_Erase(void)
{
	int i = 0;
	for (i = APPLICATION_START_SECTOR; i <= APPLICATION_END_SECTOR; i += (FLASH_Sector_1-FLASH_Sector_0))
	{
		/* Device voltage range supposed to be [2.7V to 3.6V], the operation will
		be done by word */ 
		if (FLASH_EraseSector(i, VoltageRange_3) != FLASH_COMPLETE)
		{
			/* Error occurred while page erase */
			return 1;
		}
	}

	return 0;
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

	for (i = 0; (i < DataLength) && (*FlashAddress <= (APPLICATION_END_ADDRESS-4)); i++)
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

uint16_t FLASH_Addr_To_Sector(uint32_t addr)
{
   if (addr < ADDR_FLASH_SECTOR_0)
      return 0xFFFF;
   if (addr < ADDR_FLASH_SECTOR_1)
      return FLASH_Sector_0;
   if (addr < ADDR_FLASH_SECTOR_2)
      return FLASH_Sector_1;
   if (addr < ADDR_FLASH_SECTOR_3)
      return FLASH_Sector_2;
   if (addr < ADDR_FLASH_SECTOR_4)
      return FLASH_Sector_3;
   if (addr < ADDR_FLASH_SECTOR_5)
      return FLASH_Sector_4;
   if (addr < ADDR_FLASH_SECTOR_6)
      return FLASH_Sector_5;
   if (addr < ADDR_FLASH_SECTOR_7)
      return FLASH_Sector_6;
   if (addr < ADDR_FLASH_SECTOR_8)
      return FLASH_Sector_7;
   if (addr < ADDR_FLASH_SECTOR_9)
      return FLASH_Sector_8;
   if (addr < ADDR_FLASH_SECTOR_10)
      return FLASH_Sector_9;
   if (addr < ADDR_FLASH_SECTOR_11)
      return FLASH_Sector_10;
   if (addr < ADDR_FLASH_SECTOR_12)
      return FLASH_Sector_11;
   if (addr < ADDR_FLASH_SECTOR_13)
      return FLASH_Sector_12;
   if (addr < ADDR_FLASH_SECTOR_14)
      return FLASH_Sector_13;
   if (addr < ADDR_FLASH_SECTOR_15)
      return FLASH_Sector_14;
   if (addr < ADDR_FLASH_SECTOR_16)
      return FLASH_Sector_15;
   if (addr < ADDR_FLASH_SECTOR_17)
      return FLASH_Sector_16;
   if (addr < ADDR_FLASH_SECTOR_18)
      return FLASH_Sector_17;
   if (addr < ADDR_FLASH_SECTOR_19)
      return FLASH_Sector_18;
   if (addr < ADDR_FLASH_SECTOR_20)
      return FLASH_Sector_19;
   if (addr < ADDR_FLASH_SECTOR_21)
      return FLASH_Sector_20;
   if (addr < ADDR_FLASH_SECTOR_22)
      return FLASH_Sector_21;
   if (addr < ADDR_FLASH_SECTOR_23)
      return FLASH_Sector_22;
   if (addr < ADDR_FLASH_SECTOR_24)
      return FLASH_Sector_23;
      
   return 0xFFFF;
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
