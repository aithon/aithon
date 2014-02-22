#ifndef AITHON_H
#define AITHON_H

// ChibiOS includes
#include "ch.h"
#include "hal.h"
#include "chprintf.h"

// ext includes
#include <string.h>
#include "ff.h"

// local includes
#include "aithon_private.h"
#include "music.h"
#include "aiconf.h"
#include "flash_if.h"


// Global constants and macros
#define NUM_DIGITAL_PINS   17
#define NUM_MOTORS         2


// general Aithon functions / definitions
void led_on(int num);
void led_off(int num);
void led_toggle(int num);
bool_t button_get(int num);
void button_wait(int num);


// wrappers around the appropriate ChibiOS delay/sleep functions
#define delayS(s) chThdSleepSeconds(s)  
#define delayMs(ms) chThdSleepMilliseconds(ms)
#define delayUs(us) halPolledDelay(US2RTT(us)) // must use busy loops for <1ms delays


// LCD functions / definitions
void lcd_on(void);
void lcd_off(void);
void lcd_clear(void);
void lcd_cursor(uint8_t row, uint8_t col);
void lcd_printChar(char data);
#define lcd_printf(fmt, ...) chprintf(&LCD, fmt, ##__VA_ARGS__)
void scroll_init(void);
void scrollSetCursor(int row, int col);
void scrollSetDelay(int delay);
void scrollEnable(void);
void scrollPause(void);
void scrollExit(void);
void scrollMessage(char* m, int row, int col, int window);

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

IMUReading imu_getReading(IMUDevice device);
int8_t imu_getTemp(void);


// motor functions / definitions
void motor_set(int motor, int power);
void motor_brake(int motor, int power);


// digital pin functions / definitions
typedef enum {
   INPUT             = PAL_MODE_INPUT,
   INPUT_PULLUP      = PAL_MODE_INPUT_PULLUP,
   INPUT_PULLDOWN    = PAL_MODE_INPUT_PULLDOWN,
   OUTPUT            = PAL_MODE_OUTPUT_PUSHPULL,
   OUTPUT_OPENDRAIN  = PAL_MODE_OUTPUT_OPENDRAIN,
} DigitalMode;
#define DIGITAL_LOW 		PAL_LOW
#define DIGITAL_HIGH 	PAL_HIGH

void digital_mode(int pin, DigitalMode mode);
int digital_get(int pin);
void digital_set(int pin, int value);
void digital_toggle(int pin);


// analog pin functions / definitions
typedef enum {
   MAIN_PWR = 10,
   SERVO_PWR = 11,
} AnalogPin;
uint16_t analog_get(AnalogPin pin);


// servo pin funtions / definitions
void servo_enableAll(void);
void servo_disableAll(void);
void servo_set(int pin, int position);


// music functions / definitions
void music_playNote(MusicNote note);
void music_playNotes(MusicNote *notes, int numNotes);
void music_playNotesAsync(MusicNote *notes, int numNotes);
#define music_playSong(song) music_playNotes(song, sizeof(song)/sizeof(MusicNote))
#define music_playSongAsync(song) music_playNotesAsync(song, sizeof(song)/sizeof(MusicNote))


// USB device - virtual COM port functions / definitions
void _usbcdc_init(void);
#define usbcdc_get() chSequentialStreamGet(&SDU1)
#define usbcdc_put(b) chSequentialStreamPut(&SDU1, b)
#define usbcdc_printf(fmt, ...) chprintf((BaseSequentialStream *)&SDU1, fmt, ##__VA_ARGS__)


// microSD functions / definitions
extern FATFS sdFS;
bool_t sd_init(void);


// emulated EEPROM functions
uint16_t ee_get(uint8_t virtAddr, uint16_t* data);
uint16_t ee_put(uint8_t virtAddr, uint16_t data);

#endif
