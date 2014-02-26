#include "Aithon.h"
#include "flash_if.h"

// general constants
#define PACKET_LEN         1024
#define DEFAULT_TIMEOUT    1000

// function pointer used for jumping to the user program
typedef void (*funcPtr)(void);

// all the interfaces that programming is supported over
#define NUM_INTERFACES  3
SerialDriver *_interfaces[NUM_INTERFACES] = {&SD1, &SD2, (SerialDriver *)&SDU1};
SerialDriver *_interface;

uint8_t _buffer[PACKET_LEN];
