#include "Aithon.h"


/**
 * @file    Aithon.c
 * @brief   Miscellaneous Aithon library functions
 */

static WORKING_AREA(waResetThread, 128);
static msg_t ResetThread(void *arg)
{
	(void)arg;
   chRegSetThreadName("Serial Monitor");
   while (1)
   {
		if (sdGetTimeout(&SD1, TIME_IMMEDIATE) == 0x11 || sdGetTimeout(&SD2, TIME_IMMEDIATE) == 0x11)
		{
			NVIC_SystemReset();
      }
#if AI_USE_USB_CDC
		if (sdGetTimeout(&SDU1, 100) == 0x11)
		{
			NVIC_SystemReset();
      }
#endif
      chThdSleepMilliseconds(50);
   }
	return 0;
}

/**
 * @brief   Aithon initialization.
 * @details This function initializes all Aithon libraries as well as ChibiOS.
 *          It should be called before any other code in your main function
 *          and should only be called once.
 */
void aiInit(void)
{
   // initialize ChibiOS
   halInit();
   chSysInit();
   
   // initialize the various Aithon library functions
#if AI_USE_LCD
   aiLCDInit();
#endif
#if AI_USE_MOTORS
   aiMotorInit();
#endif
#if AI_USE_SERVOS
   aiServoInit();
#endif
#if AI_USE_MUSIC
   aiMusicInit();
#endif
#if AI_USE_USB_CDC
   aiUSBCDCInit();
#endif
#if AI_USE_IMU
   aiIMUInit();
#endif
#if AI_USE_ANALOG
   aiAnalogInit();
#endif
	
   // Creates a worker thread to monitor for a new program
   sdStart(&SD1, NULL);
   sdStart(&SD2, NULL);
   chThdCreateStatic(waResetThread, sizeof(waResetThread), HIGHPRIO, ResetThread, NULL);
   
   // setup LEDs as off
   aiLEDOff(0);
   aiLEDOff(1);
}


/**
 * @addtogroup LED
 * @brief   Functions for controlling the on-boad LEDs
 * @{
 */

/**
 * @brief   Turn on LED
 * @details Turns on one of the two on-board user LEDs (0 or 1).
 *
 * @param[in] num    Which LED to turn on. Either 0 or 1.
 */
void aiLEDOn(int num)
{
   if (num == 0)
   {
      palSetPad(GPIOE, GPIOE_LED0);
   }
   else if (num == 1)
   {
      palSetPad(GPIOD, GPIOD_LED1);
   }
}

/**
 * @brief   Turn off LED
 * @details Turns off one of the two on-board user LEDs (0 or 1).
 *
 * @param[in] num    Which LED to turn off. Either 0 or 1.
 */
void aiLEDOff(int num)
{
   if (num == 0)
   {
      palClearPad(GPIOE, GPIOE_LED0);
   }
   else if (num == 1)
   {
      palClearPad(GPIOD, GPIOD_LED1);
   }
}

/**
 * @brief   Toggle LED
 * @details Toggles one of the two on-board user LEDs (0 or 1).
 *
 * @param[in] num    Which LED to toggle. Either 0 or 1.
 */
void aiLEDToggle(int num)
{
   if (num == 0)
   {
      palTogglePad(GPIOE, GPIOE_LED0);
   }
   else if (num == 1)
   {
      palTogglePad(GPIOD, GPIOD_LED1);
   }
}

/** @} */


/**
 * @addtogroup Button
 * @brief   Functions for the on-board buttons.
 * @{
 */

static const int buttons[2] = {GPIOD_BTN0, GPIOD_BTN1};


/**
 * @brief   Gets the status of one of the on-board push-buttons.
 *
 * @param[in] num    Which button to get the status of. Either 0 or 1.
 * @retval TRUE      The button is pressed.
 * @retval FALSE     The button is not pressed.
 */
bool_t aiGetButton(int num)
{
   return !palReadPad(GPIOD, buttons[num]);
}

/**
 * @brief   Blocks until a button is pressed and then released.
 *
 * @param[in] num    Which button to wait for. Either 0 or 1.
 */
void aiButtonWait(int num)
{
   // wait for button to be pushed down
	while (!aiGetButton(num))
      chThdSleepMilliseconds(1);
	// delay 30 ms for button debouncing
	chThdSleepMilliseconds(30);
	// wait for button to be released, if it is still down
	while (aiGetButton(num))
      chThdSleepMilliseconds(1);
	// delay 30 ms for button debouncing
	chThdSleepMilliseconds(30);
}

/** @} */