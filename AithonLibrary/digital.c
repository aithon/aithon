#include "Aithon.h"


/**
 * @file    digital.c
 * @brief   Digital pin functions
 *
 * @addtogroup Digital
 * @brief   Functions for configuring, setting, and reading digital I/O pins.
 * @{
 */

GPIO_TypeDef *_ports[17] = {
   GPIOC, // 0
   GPIOC, // 1
   GPIOC, // 2
   GPIOE, // 3
   GPIOE, // 4
   GPIOE, // 5
   GPIOE, // 6
   GPIOE, // 7
   GPIOE, // 8
   GPIOE, // 9
   GPIOB, // 10
   GPIOB, // 11
   GPIOB, // 12
   GPIOD, // 13
   GPIOD, // 14
   GPIOE, // 15
   GPIOE, // 16
};

int _pads[17] = {
   GPIOC_DIGITAL0,
   GPIOC_DIGITAL1,
   GPIOC_DIGITAL2,
   GPIOE_DIGITAL3,
   GPIOE_DIGITAL4,
   GPIOE_DIGITAL5,
   GPIOE_DIGITAL6,
   GPIOE_DIGITAL7,
   GPIOE_DIGITAL8,
   GPIOE_DIGITAL9,
   GPIOB_DIGITAL10,
   GPIOB_DIGITAL11,
   GPIOB_DIGITAL12,
   GPIOD_DIGITAL13,
   GPIOD_DIGITAL14,
   GPIOE_DIGITAL15,
   GPIOE_DIGITAL16
};


/**
 * @brief   Sets a digital pin's mode
 * @details This function changes the current mode of a digital I/O pin.
 *
 * @param[in] pin    Which digital pin to set the mode for (0-14).
 * @param[in] mode   The DigitalMode to configure this pin as.
 */
void aiDigitalMode(int pin, DigitalMode mode)
{
   palSetPadMode(_ports[pin], _pads[pin], mode);
}

/**
 * @brief   Reads a digital pin
 * @details This function gets the current value of a digital I/O pin.
 * @note    The behavior is undefined if the pin is not configured as an input
 *          pin.
 *
 * @param[in] pin    Which digital pin to get the value of (0-14).
 * @retval PAL_LOW   The pin is in the low state.
 * @retval PAL_HIGH  The pin is in the high state.
 */
int aiDigitalIn(int pin)
{
   return palReadPad(_ports[pin], _pads[pin]);
}

/**
 * @brief   Sets a digital pin
 * @details This function sets the value of a digital I/O pin.
 * @note    The behavior is undefined if the pin is not configured as an output
 *          pin.
 *
 * @param[in] pin    Which digital pin to get the value of (0-14).
 * @param[in] value  Which digital pin to get the value of (0-14).
 */
void aiDigitalOut(int pin, int value)
{
   palWritePad(_ports[pin], _pads[pin], value);
}

/**
 * @brief   Toggles a digital pin
 * @details This function toggles the value of a digital I/O pin.
 * @note    The behavior is undefined if the pin is not configured as an output
 *          pin.
 *
 * @param[in] pin    Which digital pin to toggle (0-14).
 */
void aiDigitalToggle(int pin)
{
   palTogglePad(_ports[pin], _pads[pin]);
}

/** @} */