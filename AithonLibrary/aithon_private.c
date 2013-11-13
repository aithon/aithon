#include "Aithon.h"

/**
 * @file    aithon_private.c
 * @brief   Private Aithon library functions - should NOT be used by user-applications.
 */


void _aiBKSRAMInit(void)
{
   RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;
   PWR->CSR |= PWR_CSR_BRE;
   while ((PWR->CSR & PWR_CSR_BRR) == 0);
}

void _aiResetToBootloader(void)
{
   _aiBKSRAMInit();
   _AI_RESERVED_BYTE |= 0x02; // the bootloader should be run
   NVIC_SystemReset();
}