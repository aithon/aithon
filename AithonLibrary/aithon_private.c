#include "Aithon.h"


extern int __flash_start__;
extern void ResetHandler(void);
__attribute__((section(".flash_size"))) __attribute__((used)) int __dummy = (int)&__flash_start__;

/**
 * @file    aithon_private.c
 * @brief   Private Aithon library functions - should NOT be called by user-applications.
 */

void __late_init(void)
{
   // Need use __dummy because for some reason the used attribute doesn't work
   // and the compiler will remove this variable if it's not used. So, we'll use
   // it to do a simple sanity check that the program is in the right place.
   if (*((uint32_t *)(__dummy+4)) != (uint32_t)ResetHandler)
      NVIC_SystemReset();
   
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
   _ee_init();
   _usbcdc_init();
   _lcd_init();

   // setup LEDs as off
   led_off(0);
   led_off(1);

   // setup serial drivers for UART1 and UART2
   sdStart(&SD1, NULL);
   sdStart(&SD2, NULL);

#ifndef _AI_IS_BOOTLOADER
   _aithon_private_init();
#endif
}

void _default_exit(void)
{
   chThdSleep(TIME_INFINITE);
}


void _reset_to_bootloader(void)
{
   uint16_t data = 0xFFFF;
   _ee_getReserved(_AI_EE_RES_BOOT_OPTIONS, &data);
   data = (data & 0xFFF) | 0xA000;
   _ee_putReserved(_AI_EE_RES_BOOT_OPTIONS, data);
   NVIC_SystemReset();
}


Thread *_aithon_thd = 0;
static WORKING_AREA(_waAithonThread, 128);
static msg_t aithon_thread(void *p)
{
   (void)p;
   eventmask_t evt = chEvtWaitAny(ALL_EVENTS);
   if (evt == 1)
   {
      // Run USB cleanup code here
      _usb_stop();
   }
   else
   {
      // Run UART cleanup code here
   }
   _reset_to_bootloader();
   return 0;
}

void _aithon_private_init(void)
{
   _aithon_thd = chThdCreateStatic(_waAithonThread, sizeof(_waAithonThread), ABSPRIO, aithon_thread, NULL);
}