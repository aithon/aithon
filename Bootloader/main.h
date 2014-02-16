#include "Aithon.h"
#include "flash_if.h"

// general constants
#define PACKET_LEN         1024
#define BOOT_TIMEOUT       30000
#define DEFAULT_TIMEOUT    100

// control characters
#define SYNC               0xA5
#define BUSY               0xB2
#define ACK                0x79
#define NACK               0x1F
#define ERASE_FLASH_START  0x43
#define ERASE_FLASH_STATUS 0x8C
#define SET_ADDR           0x31
#define FILL_BUFFER        0xC7
#define COMMIT_BUFFER      0x6E
#define START_PROGRAM      0x2A


// function pointer used for jumping to the user program
typedef void (*funcPtr)(void);

// all the interfaces that programming is supported over
#define NUM_INTERFACES  3
SerialDriver *_interfaces[NUM_INTERFACES] = {&SD1, &SD2, (SerialDriver *)&SDU1};
SerialDriver *_interface;

uint8_t _buffer[PACKET_LEN];