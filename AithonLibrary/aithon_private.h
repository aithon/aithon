#ifndef AITHON_PRIVATE_H
#define AITHON_PRIVATE_H

// Private Aithon functions / definitions


void _aiPrivateInit(void);


// reserved EEPROM addresses
#define _AI_EE_RES_ADDR_BOOT			0x00 // should bootloader run

// reserved EEPROM values
#define _AI_EE_RES_VAL_BOOT_RUN		0xABCD // bootloader should run
#define _AI_EE_RES_VAL_DEfAULT		0x00 // default value


#define _AI_RESET_CMD					0xA5 // reset command if programming over UART


void _aiResetToBootloader(void);
uint16_t _aiEEWriteReserved(uint8_t resAddr, uint16_t data);
uint16_t _aiEEReadReserved(uint8_t resAddr, uint16_t *data);

#endif