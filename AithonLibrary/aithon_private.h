#ifndef AITHON_PRIVATE_H
#define AITHON_PRIVATE_H

// Private Aithon functions / definitions

// We use the last byte of the backup SRAM to store temporary data across a software reset
// for the bootloader. The bits are used for:
// 0 		- 	Reserved
// 1 		- 	Whether we should run the bootloader or not
// 2-7	- 	Reserved
#define _AI_RESERVED_BYTE (*((uint8_t *)((BKPSRAM_BASE+4095))))
void _aiBKSRAMInit(void);
void _aiResetToBootloader(void);

#endif