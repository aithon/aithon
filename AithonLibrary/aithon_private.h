#ifndef AITHON_PRIVATE_H
#define AITHON_PRIVATE_H

// Private Aithon functions / definitions

extern BaseSequentialStream LCD;
void _lcd_init(void);
void _imu_init(void);
void _motor_init(void);
void _analog_init(void);
void _servo_init(void);
void _music_init(void);
extern SerialUSBDriver SDU1;
void _usb_init(void);
uint16_t _ee_init(void);
void __late_init(void);
void _aiPrivateInit(void);


// reserved EEPROM addresses
#define _AI_EE_RES_ADDR_BOOT			0x00 // should bootloader run

// reserved EEPROM values
#define _AI_EE_RES_VAL_BOOT_RUN		0xABCD // bootloader should run
#define _AI_EE_RES_VAL_DEfAULT		0x00 // default value


#define _AI_RESET_CMD					0xA5 // reset command if programming over UART


void _reset_to_bootloader(void);
uint16_t _ee_putReserved(uint8_t resAddr, uint16_t data);
uint16_t _ee_getReserved(uint8_t resAddr, uint16_t *data);

#endif