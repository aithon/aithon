#include "Aithon.h"

/**
 * @file    Aithon.c
 * @brief   Miscellaneous Aithon library functions
 */

// FATFS Object
FATFS sdFS;

MMCDriver MMCD1;
// Maximum speed SPI configuration (18MHz, CPHA=0, CPOL=0, MSb first).
static SPIConfig hs_spicfg = {NULL, GPIOD, GPIOD_SPI3_NSS, 0};
// Low speed SPI configuration (281.250kHz, CPHA=0, CPOL=0, MSb first).
static SPIConfig ls_spicfg = {NULL, GPIOD, GPIOD_SPI3_NSS, SPI_CR1_BR_2 | SPI_CR1_BR_1};
// MMC/SD over SPI driver configuration.
static MMCConfig mmccfg = {&SPID3, &ls_spicfg, &hs_spicfg};


/**
 * @brief   microSD Card initialization.
 * @details This function initializes the microSD card interface
            using an mcc over SPI protocol. This should only be
            called if there is a microSD card in the socket.
            This function will also mount the root directory to
            the sdFS file system object.
 * @retval TRUE      There was an error.
 * @retval FALSE     No error
 */
bool_t aiSDInit(void)
{
   // TODO: Check that there is a card in the microSD socket

   mmcObjectInit(&MMCD1);
   mmcStart(&MMCD1, &mmccfg);
   if (mmcConnect(&MMCD1))
      return TRUE;

   if (f_mount(0, &sdFS) != FR_OK)
   {
      mmcDisconnect(&MMCD1);
      return TRUE;
   }

   return FALSE;
}

/** @} */