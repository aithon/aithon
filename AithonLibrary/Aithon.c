#include "Aithon.h"


/**
 * @file    Aithon.c
 * @brief   Miscellaneous Aithon library functions
 */


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
#ifndef _AI_IS_BOOTLOADER
   aiMotorInit();
   aiServoInit();
   aiMusicInit();
   aiIMUInit();
   aiAnalogInit();
#endif
   aiLCDInit();
   aiEEInit();
   aiUSBCDCInit();

   // setup LEDs as off
   aiLEDOff(0);
   aiLEDOff(1);

   // setup serial drivers for UART1 and UART2
   sdStart(&SD1, NULL);
   sdStart(&SD2, NULL);

#ifndef _AI_IS_BOOTLOADER
   _aiPrivateInit();
#endif
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