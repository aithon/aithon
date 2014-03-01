#include "Aithon.h"
#include "flash_if.h"

// general constants
#define PACKET_LEN         1024
#define BOOT_TIMEOUT       30000
#define DEFAULT_TIMEOUT    1000
#define BOOT_MAGIC         0xA
#define DEMO_START_ADDR    0x08010000
#define USER_START_ADDR    0x08020000


// Commands
#define SYNC               0x01
#define SET_START_ADDR     0x2A
#define SET_PROG_LEN       0x07
#define ERASE_FLASH_START  0x1B
#define ERASE_FLASH_STATUS 0x09
#define SET_ADDR           0x1A
#define CHECK_ADDR         0x3B
#define FILL_BUFFER        0x29
#define CHECK_BUFFER       0x06
#define COMMIT_BUFFER      0x28
#define START_PROGRAM      0x11

// Responses
#define ACK                0x40
#define NACK               0x80
#define BUSY               0xC0


// function pointer used for jumping to the user program
typedef void (*funcPtr)(void);

// all the interfaces that programming is supported over
#define NUM_INTERFACES  3
SerialDriver *_interfaces[NUM_INTERFACES] = {&SD1, &SD2, (SerialDriver *)&SDU1};
SerialDriver *_interface;

uint8_t _buffer[PACKET_LEN];