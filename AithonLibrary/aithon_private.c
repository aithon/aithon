#include "Aithon.h"

/**
 * @file    aithon_private.c
 * @brief   Private Aithon library functions - should NOT be called by user-applications.
 */


void __late_init(void)
{
   // initialize ChibiOS
   halInit();
   chSysInit();

   // initialize the various Aithon library functions
#ifndef _AI_IS_BOOTLOADER
   _motor_init();
   _servo_init();
   _music_init();
   _imu_init();
   _analog_init();
#endif
   _lcd_init();
   _ee_init();
   _usbcdc_init();

   // setup LEDs as off
   led_off(0);
   led_off(1);

   // setup serial drivers for UART1 and UART2
   sdStart(&SD1, NULL);
   sdStart(&SD2, NULL);

#ifndef _AI_IS_BOOTLOADER
   _aiPrivateInit();
#endif
}


void _reset_to_bootloader(void)
{
   _ee_putReserved(_AI_EE_RES_ADDR_BOOT, _AI_EE_RES_VAL_BOOT_RUN);
   NVIC_SystemReset();
}


#if AI_USE_UART1_PROGRAMMING || AI_USE_UART2_PROGRAMMING
static WORKING_AREA(_waResetThread, 128);
static msg_t _resetThread(void *p)
{
   (void)p;
   EventListener evtListener;
#if AI_USE_UART1_PROGRAMMING
   chEvtRegister(chnGetEventSource(&SD1), &evtListener, 1);
#endif
#if AI_USE_UART2_PROGRAMMING
   chEvtRegister(chnGetEventSource(&SD2), &evtListener, 2);
#endif
   while (TRUE) {
      eventmask_t evt = chEvtWaitAny(EVENT_MASK(1) | EVENT_MASK(2));
#if AI_USE_UART1_PROGRAMMING
      if (evt == EVENT_MASK(1) &&
          (chEvtGetAndClearFlags(&evtListener) & CHN_INPUT_AVAILABLE) &&
          (*(SD1.iqueue.q_rdptr) == _AI_RESET_CMD))
      {
         _reset_to_bootloader();
      }
#endif
#if AI_USE_UART2_PROGRAMMING
      if (evt == EVENT_MASK(2) &&
          (chEvtGetAndClearFlags(&evtListener) & CHN_INPUT_AVAILABLE) &&
          (*(SD2.iqueue.q_rdptr) == _AI_RESET_CMD))
      {
         _reset_to_bootloader();
      }
#endif
   }
   return 0;
}
#endif

void _aiPrivateInit(void)
{
#if AI_USE_UART1_PROGRAMMING || AI_USE_UART2_PROGRAMMING
   (void)chThdCreateStatic(_waResetThread, sizeof(_waResetThread), HIGHPRIO, _resetThread, NULL);
#endif
}