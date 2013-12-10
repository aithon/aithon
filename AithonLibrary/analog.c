#include "Aithon.h"


/**
 * @file    analog.c
 * @brief   ADC functions
 *
 * @addtogroup Analog
 * @brief   Functions for reading analog values
 * @{
 */

/**
 * @brief   How many ADC channels are in use
 * @note    This should not be changed unless all impacted code is
 *          appropriately changed.
 */
#define _NUM_CHANNELS   12

/**
 * @brief   How many ADC samples per channel
 * @note    This determines how many samples are stored for each
 *          channel and later averaged together.
 */
#define _NUM_SAMPLES    100

/*
   How the analog pins are connected to the ADC inputs:
   M1_SENSE    IN9
   M0_SENSE    IN8
   SERVO_PWR   IN15
   MAIN_PWR    IN14
   A7          IN10
   A6          IN11
   A5          IN12
   A4          IN13
   A3          IN0
   A2          IN1
   A1          IN2
   A0          IN3
*/


static adcsample_t _samples[_NUM_CHANNELS*_NUM_SAMPLES];

/*
 * ADC conversion group.
 * Mode:        Continuous, 100 samples of 10 channels.
 * Channels:    IN3, IN2, IN1, IN0, IN13, IN12, IN11, IN10, IN14, IN15, IN8, IN9
 */
static const ADCConversionGroup _config = {
   TRUE,
   _NUM_CHANNELS,
   NULL,
   NULL,
   0,                        /* CR1 */
   ADC_CR2_SWSTART,          /* CR2 */
   ADC_SMPR1_SMP_AN10(ADC_SAMPLE_56) | ADC_SMPR1_SMP_AN11(ADC_SAMPLE_56) |
   ADC_SMPR1_SMP_AN12(ADC_SAMPLE_56) | ADC_SMPR1_SMP_AN13(ADC_SAMPLE_56) |
   ADC_SMPR1_SMP_AN14(ADC_SAMPLE_56) | ADC_SMPR1_SMP_AN15(ADC_SAMPLE_56),
   ADC_SMPR2_SMP_AN0(ADC_SAMPLE_56) | ADC_SMPR2_SMP_AN1(ADC_SAMPLE_56) |
   ADC_SMPR2_SMP_AN2(ADC_SAMPLE_56) | ADC_SMPR2_SMP_AN3(ADC_SAMPLE_56) |
   ADC_SMPR2_SMP_AN8(ADC_SAMPLE_56) | ADC_SMPR2_SMP_AN9(ADC_SAMPLE_56),
   ADC_SQR1_NUM_CH(_NUM_CHANNELS),
   ADC_SQR2_SQ7_N(ADC_CHANNEL_IN11) | ADC_SQR2_SQ8_N(ADC_CHANNEL_IN10) |
   ADC_SQR2_SQ9_N(ADC_CHANNEL_IN14) | ADC_SQR2_SQ10_N(ADC_CHANNEL_IN15) |
   ADC_SQR2_SQ11_N(ADC_CHANNEL_IN8) | ADC_SQR2_SQ12_N(ADC_CHANNEL_IN9),
   ADC_SQR3_SQ1_N(ADC_CHANNEL_IN3) | ADC_SQR3_SQ2_N(ADC_CHANNEL_IN2) |
   ADC_SQR3_SQ3_N(ADC_CHANNEL_IN1) | ADC_SQR3_SQ4_N(ADC_CHANNEL_IN0) |
   ADC_SQR3_SQ5_N(ADC_CHANNEL_IN13) | ADC_SQR3_SQ6_N(ADC_CHANNEL_IN12)
};


/**
 * @brief   Analog function initialization.
 * @details This function sets up and starts the ADC conversions for the
 *          various analog inputs on the Aithon board. This includes all 8
 *          analog input pins, the motor current sensors, and the main and
 *          servo input voltages.
 * @note    This function is called by aiInit() and does not need to be called
 *          again by the user after aiInit() is called.
 */
void _analog_init(void)
{
   adcStart(&ADCD1, NULL);
   adcStartConversion(&ADCD1, &_config, _samples, _NUM_SAMPLES);
}

/**
 * @brief   Get value of an analog input
 * @details This function returns the average of the last 100 ADC readings on
 *          an analog input pin.
 *
 * @param[in] pin    Which pin to get the reading of.
 * @return           The 12-bit (0-4095) value of the average ADC reading.
 */
uint16_t analog_get(AnalogPin pin)
{
   int i, total = 0;
   for (i = 0; i < _NUM_SAMPLES; i++)
   {
      total += _samples[i*_NUM_CHANNELS+pin];
   }
   return (uint16_t) (total/_NUM_SAMPLES);
}

/** @} */