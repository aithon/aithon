#include "Aithon.h"

static const int motorPins[NUM_MOTORS][2] = {
   {GPIOE_MOTOR0_B, GPIOE_MOTOR0_A},
   {GPIOE_MOTOR1_A, GPIOE_MOTOR1_B}
};

static const PWMConfig pwmcfg1 = {
	3000000,
	100,
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
   palSetPadMode(GPIOE, GPIOE_MOTOR0_EN, PAL_MODE_ALTERNATE(1));
   palSetPadMode(GPIOE, GPIOE_MOTOR1_EN, PAL_MODE_ALTERNATE(1));
   palSetPadMode(GPIOE, GPIOE_MOTOR0_A, PAL_MODE_OUTPUT_PUSHPULL);
   palSetPadMode(GPIOE, GPIOE_MOTOR1_A, PAL_MODE_OUTPUT_PUSHPULL);
   palSetPadMode(GPIOE, GPIOE_MOTOR0_B, PAL_MODE_OUTPUT_PUSHPULL);
   palSetPadMode(GPIOE, GPIOE_MOTOR1_B, PAL_MODE_OUTPUT_PUSHPULL);
   pwmStart(&PWMD1, &pwmcfg1);
}

void motor_set(int motor, int power)
{
   motor = (motor + 1) % 2; // The motor connections are currently swapped on the board
   if (power > 0)
   {
      pwmEnableChannel(&PWMD1, motor, power);
      palWritePad(GPIOE, motorPins[motor][0], 1);
      palWritePad(GPIOE, motorPins[motor][1], 0);
   }
   else if (power < 0)
   {
      pwmEnableChannel(&PWMD1, motor, -power);
      palWritePad(GPIOE, motorPins[motor][0], 0);
      palWritePad(GPIOE, motorPins[motor][1], 1);
   }
   else
   {
      pwmEnableChannel(&PWMD1, motor, 0);
      palWritePad(GPIOE, motorPins[motor][0], 0);
      palWritePad(GPIOE, motorPins[motor][1], 0);
   }
}

void motor_brake(int motor, int power)
{
   motor = (motor + 1) % 2; // The motor connections are currently swapped on the board
   palWritePad(GPIOE, motorPins[motor][0], 0);
   palWritePad(GPIOE, motorPins[motor][1], 0);
   pwmEnableChannel(&PWMD1, motor, power*100);
}