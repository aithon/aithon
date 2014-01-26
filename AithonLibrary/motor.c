#include "Aithon.h"

static const int motorPins[NUM_MOTORS][2] = {
   {GPIOE_MOTOR0_B, GPIOE_MOTOR0_A},
   {GPIOE_MOTOR1_A, GPIOE_MOTOR1_B}
};

static const PWMConfig pwmcfg1 = {
	10000,
	1,
	NULL,
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_DISABLED, NULL},
		{PWM_OUTPUT_DISABLED, NULL},
	},
	0,
	0,
   0
};

void _motor_init(void)
{
   pwmStart(&PWMD1, &pwmcfg1);
}

void motor_set(int motor, int power)
{
   if (power > 0)
   {
      palWritePad(GPIOE, motorPins[motor][0], 1);
      palWritePad(GPIOE, motorPins[motor][1], 0);
   }
   else if (power < 0)
   {
      palWritePad(GPIOE, motorPins[motor][0], 0);
      palWritePad(GPIOE, motorPins[motor][1], 1);
   }
   else
   {
      palWritePad(GPIOE, motorPins[motor][0], 0);
      palWritePad(GPIOE, motorPins[motor][1], 0);
   }
   pwmEnableChannel(&PWMD1, motor, power);
}

void motor_brake(int motor, int power)
{
   palWritePad(GPIOE, motorPins[motor][0], 0);
   palWritePad(GPIOE, motorPins[motor][1], 0);
   pwmEnableChannel(&PWMD1, motor, power);
}