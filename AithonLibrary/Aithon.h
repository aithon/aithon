#ifndef AITHON_H
#define AITHON_H

// ChibiOS includes
#include "ch.h"
#include "hal.h"
#include "music.h"
#include "aiconf.h"

#if AI_USE_LCD
#include "chprintf.h"
#endif

#if AI_USE_USB_CDC
#include "usb_cdc.h"
#endif


// general Aithon functions / definitions
void aiInit(void);
void aiLEDOn(int num);
void aiLEDOff(int num);
void aiLEDToggle(int num);
bool_t aiGetButton(int num);
void aiButtonWait(int num);


// wrappers around the appropriate ChibiOS delay/sleep functions
#define aiDelayS(s) chThdSleepSeconds(s)  
#define aiDelayMs(ms) chThdSleepMicroseconds(ms)
#define aiDelayUs(us) halPolledDelay(US2RTT(us)) // must use busy loops for <1ms delays


#if AI_USE_LCD
// LCD functions / definitions
void aiLCDInit(void);
void aiLCDClear(void);
void aiLCDTopLine(void);
void aiLCDBottomLine(void);
extern BaseSequentialStream LCD;
#define aiLCDPrintf(fmt, ...) chprintf(&LCD, fmt, ##__VA_ARGS__)
void aiLCDPrintChar(const char data);
#endif


#if AI_USE_IMU
// IMU functions / definitions
typedef enum {
   ACCEL,
   GYRO
} IMUDevice;

typedef struct {
   int16_t x;
   int16_t y;
   int16_t z;
} IMUReading;

void aiIMUInit(void);
IMUReading aiIMUGetReading(IMUDevice dev);
int8_t aiIMUGetTemp(void);
#endif


#if AI_USE_MOTORS
// motor functions / definitions
void aiMotorInit(void);
void aiMotorSet(int num, int power);
void aiMotorBrake(int num, int power);
float aiMotorCurrent(int num);
#endif


// digital pin functions / definitions
typedef enum {
   INPUT             = PAL_MODE_INPUT,
   INPUT_PULLUP      = PAL_MODE_INPUT_PULLUP,
   INPUT_PULLDOWN    = PAL_MODE_INPUT_PULLDOWN,
   OUTPUT            = PAL_MODE_OUTPUT_PUSHPULL,
   OUTPUT_OPENDRAIN  = PAL_MODE_OUTPUT_OPENDRAIN,
} DigitalMode;

void aiDigitalMode(int pin, DigitalMode mode);
int aiDigitalIn(int pin);
void aiDigitalOut(int pin, int value);
void aiDigitalToggle(int pin);


// analog pin functions / definitions
typedef enum {
   MAIN_PWR = 8,
   SERVO_PWR = 9,
   M0_SENSE = 10,
   M1_SENSE = 11,
} AnalogPin;
void aiAnalogInit(void);
uint16_t aiAnalogInput(AnalogPin pin);


#if AI_USE_SERVOS
// servo pin funtions / definitions
void aiServoInit(void);
void aiServosOn(void);
void aiServosOff(void);
void aiServoOutput(int pin, int position);
#endif


#if AI_USE_MUSIC
// music functions / definitions
void aiMusicInit(void);
void aiMusicPlayNote(MusicNote note);
void aiMusicPlayNotes(MusicNote *notes, int numNotes);
void aiMusicPlayNotesAsync(MusicNote *notes, int numNotes);
#define aiMusicPlaySong(song) aiMusicPlayNotes(song, sizeof(song)/sizeof(MusicNote))
#define aiMusicPlaySongAsync(song) aiMusicPlayNotesAsync(song, sizeof(song)/sizeof(MusicNote))
#endif


#if AI_USE_USB_CDC
// USB device - virtual COM port functions / definitions
extern SerialUSBDriver SDU1;
void aiUSBCDCInit(void);
#define aiUSBCDCGet() chSequentialStreamGet(&SDU1)
#define aiUSBCDCPut(b) chSequentialStreamPut(&SDU1, b)
#define aiUSBCDCPrintf(fmt, ...) chprintf((BaseSequentialStream *)&SDU1, fmt, ##__VA_ARGS__)
#endif

#endif