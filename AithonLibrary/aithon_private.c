#include "Aithon.h"

/**
 * @file    aithon_private.c
 * @brief   Private Aithon library functions - should NOT be used by user-applications.
 */



void _aiResetToBootloader(void)
{
   _aiEEWriteReserved(_AI_EE_RES_ADDR_BOOT, _AI_EE_RES_VAL_BOOT_RUN);
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
         _aiResetToBootloader();
      }
#endif
#if AI_USE_UART2_PROGRAMMING
      if (evt == EVENT_MASK(2) &&
          (chEvtGetAndClearFlags(&evtListener) & CHN_INPUT_AVAILABLE) &&
          (*(SD2.iqueue.q_rdptr) == _AI_RESET_CMD))
      {
         _aiResetToBootloader();
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