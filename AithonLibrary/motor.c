#include "Aithon.h"

volatile int count = 0;
volatile int values[2] = {0, 0};
volatile int braking[2] = {0, 0};
static const int motorPins[2][3] = {
   {GPIOE_MOTOR0_B, GPIOE_MOTOR0_A, GPIOE_MOTOR0_EN},
   {GPIOE_MOTOR1_A, GPIOE_MOTOR1_B, GPIOE_MOTOR1_EN}
};

void callback(PWMDriver *pwmp)
{
   (void)pwmp;
   int i;
   if (++count == 100)
   {
      count = 0;
      for (i = 0; i < 2; i++)
      {
         if (!braking[i])
         {
            palWritePad(GPIOE, motorPins[i][0], (values[i]>0));
            palWritePad(GPIOE, motorPins[i][1], (values[i]<0));
         }
      }
   }
   
   for (i = 0; i < 2; i++)
   {
      int tmp = values[i]<0?values[i]*-1:values[i];
      if (tmp <= count && !braking[i])
      {
         palClearPad(GPIOE, motorPins[i][0]);
         palClearPad(GPIOE, motorPins[i][1]);
      }
   }
}

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
      palSetPad(GPIOE, motorPins[motor][2]);
      values[motor] = power;
   }
   else if (power < 0)
   {
      palWritePad(GPIOE, motorPins[motor][0], 0);
      palWritePad(GPIOE, motorPins[motor][1], 1);
      palSetPad(GPIOE, motorPins[motor][2]);
      values[motor] = power;
   }
   else
   {
      palWritePad(GPIOE, motorPins[motor][0], 0);
      palWritePad(GPIOE, motorPins[motor][1], 0);
      palClearPad(GPIOE, motorPins[motor][2]);
      values[motor] = 100;
   }
   braking[motor] = 0;
}

void motor_brake(int motor, int power)
{
   palWritePad(GPIOE, motorPins[motor][0], 0);
   palWritePad(GPIOE, motorPins[motor][1], 0);
   palSetPad(GPIOE, motorPins[motor][2]);
   braking[motor] = 1;
   values[motor] = power;
}

#ifdef AITHON_r4
float motor_getCurrent(int motor)
{
   if (motor == 0)
      return analog_get(M0_SENSE)*1.0/585.0;
   else if (motor == 1)
      return analog_get(M1_SENSE)*1.0/585.0;
      
   return -1;
}
#endif