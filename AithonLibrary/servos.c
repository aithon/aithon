#include "Aithon.h"

#if AI_USE_SERVOS

static const PWMConfig pwmcfg4 = {
	1000000,
	20000,
	NULL,
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
	},
	0
};

static const PWMConfig pwmcfg8 = {
	1000000,
	20000,
	NULL,
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
	},
	0
};

void aiServoInit(void)
{
   aiServosOn();
}

void aiServosOn(void)
{
   pwmStart(&PWMD4, &pwmcfg4);
   pwmStart(&PWMD8, &pwmcfg8);
}

void aiServosOff(void)
{
   pwmStop(&PWMD4);
   pwmStop(&PWMD8);
}

void aiServoOutput(int pin, int position)
{
   // if (position < 0 || position > 150)
   // {
      // // out of range
      // return;
   // }
   
   
   if (pin > 0 && pin < 4)
   {
      pwmEnableChannel(&PWMD4, pin, (position+75)*10);
   }
   else if (pin < 8)
   {
      pwmEnableChannel(&PWMD8, pin-4, (position+75)*10);
   }
}

#endif