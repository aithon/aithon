#include "Tester.h"

// #define DAC_BUFFER_SIZE 2

// const uint16_t dac_buffer[DAC_BUFFER_SIZE] = {0, 4096};


// static void daccb(DACDriver *dacp) {
   // (void)dacp;
   // led_on(0);
// }

// static void dacerrcb(DACDriver *dacp) {
   // (void)dacp;
   // led_on(1);
// }

// static const DACConfig daccfg1 = {
   // DAC_MODE_CONTINUOUS,
   // 400*DAC_BUFFER_SIZE, /* Multiply the buffer size to the desired frequency in Hz */
   // dac_buffer, /* Pointer to the first buffer */
   // NULL, /* Pointer to the second buffer */
   // DAC_BUFFER_SIZE, /* Buffers size */
   // daccb, /* End of transfer callback */
   // dacerrcb, /* Error callback */
   // /* STM32 specific config starts here */
   // DAC_DHRM_12BIT_RIGHT, /* data holding register mode */
   // 0 /* CR flags */
// };

int test_buzzer()
{
   // led_off(0);
   // led_off(1);
   // dacStart(&DACD1, &daccfg1);
   // dacStartSend(&DACD1);
   // delayS(10);
   // // NOT YET IMPLEMENTED
   return 0;
}