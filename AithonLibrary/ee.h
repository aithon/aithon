#ifndef EE_H
#define EE_H

#include "aithon.h"

uint16_t aiEEInit(void);
uint16_t aiEERead(uint8_t virtAddr, uint16_t* data);
uint16_t aiEEWrite(uint8_t virtAddr, uint16_t data);

#endif
