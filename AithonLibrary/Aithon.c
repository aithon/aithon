#include "Aithon.h"


/**
 * @file    Aithon.c
 * @brief   Miscellaneous Aithon library functions
 */


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
void led_on(int num)
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
void led_off(int num)
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
void led_toggle(int num)
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

static const int _buttons[2] = {GPIOD_BTN0, GPIOD_BTN1};


/**
 * @brief   Gets the status of one of the on-board push-buttons.
 *
 * @param[in] num    Which button to get the status of. Either 0 or 1.
 * @retval TRUE      The button is pressed.
 * @retval FALSE     The button is not pressed.
 */
bool_t button_get(int num)
{
   switch (num)
   {
   case 0:
      return !palReadPad(GPIOD, GPIOD_BTN0);
   case 1:
      return !palReadPad(GPIOD, GPIOD_BTN1);
   default:
      return FALSE;
   }
}

/**
 * @brief   Blocks until a button is pressed and then released.
 *
 * @param[in] num    Which button to wait for. Either 0 or 1.
 */
void button_wait(int num)
{
   // wait for button to be pushed down
   while (!button_get(num))
      chThdSleepMilliseconds(1);
   // delay 30 ms for button debouncing
   chThdSleepMilliseconds(30);
   // wait for button to be released, if it is still down
   while (button_get(num))
      chThdSleepMilliseconds(1);
   // delay 30 ms for button debouncing
   chThdSleepMilliseconds(30);
}

/** @} */