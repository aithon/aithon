#include "Aithon.h"

static const PWMConfig _pwmcfg4 = {
	1000000,
	20000,
	NULL,
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
	},
	0,
	0
};

static const PWMConfig _pwmcfg8 = {
	1000000,
	20000,
	NULL,
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
	},
	0,
	0
};

void _servo_init(void)
{
   servo_enableAll();
}

void servo_enableAll(void)
{
   pwmStart(&PWMD4, &_pwmcfg4);
   pwmStart(&PWMD8, &_pwmcfg8);
}

void servo_disableAll(void)
{
   pwmStop(&PWMD4);
   pwmStop(&PWMD8);
}

void servo_set(int servoNum, int position)
{
   // if (position < 0 || position > 150)
   // {
      // // out of range
      // return;
   // }
   
   
   if (servoNum => 0 && servoNum < 4)
   {
      pwmEnableChannel(&PWMD4, servoNum, (position+75)*10);
   }
   else if (servoNum < 8)
   {
      pwmEnableChannel(&PWMD8, servoNum-4, (position+75)*10);
   }
}
