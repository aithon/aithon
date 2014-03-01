/**
  ******************************************************************************
  * @file    STM32F4xx_IAP/inc/flash_if.h 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    10-October-2011
  * @brief   This file provides all the headers of the flash_if functions.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_IF_H
#define __FLASH_IF_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm_flash.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
   FLASH_ERASE_COMPLETE,
	FLASH_ERASE_IN_PROGRESS,
	FLASH_ERASE_ERROR
} FLASH_EraseResult;

/* Exported constants --------------------------------------------------------*/
extern const uint32_t FLASH_SECTOR_ADDR[NUM_FLASH_SECTORS+1];

#define BOOTLOADER_FIRST_SECTOR     0
#define BOOTLOADER_LAST_SECTOR      1

#define EEPROM_PAGE0_FIRST_SECTOR   2
#define EEPROM_PAGE0_LAST_SECTOR    2
#define EEPROM_PAGE1_FIRST_SECTOR   3
#define EEPROM_PAGE1_LAST_SECTOR    3

#define DEMO_FIRST_SECTOR           4
#define DEMO_LAST_SECTOR            4

#define APPLICATION_FIRST_SECTOR    5
#define APPLICATION_LAST_SECTOR     11 // this is last sector of 1MB of FLASH

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void FLASH_If_Init(void);
FLASH_EraseResult FLASH_If_Erase_Start(uint16_t startSector);
FLASH_EraseResult FLASH_If_Erase_Status(uint16_t endSector);
uint32_t FLASH_If_Write(__IO uint32_t* FlashAddress, uint32_t* Data, uint32_t DataLength);
uint16_t FLASH_Addr_To_Sector(uint32_t addr);


#endif  /* __FLASH_IF_H */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/
