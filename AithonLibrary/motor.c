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

void aiMotorInit(void)
{
   pwmStart(&PWMD1, &pwmcfg1);
}

void aiMotorSet(int num, int power)
{
   if (power > 0)
   {
      palWritePad(GPIOE, motorPins[num][0], 1);
      palWritePad(GPIOE, motorPins[num][1], 0);
      palSetPad(GPIOE, motorPins[num][2]);
      values[num] = power;
   }
   else if (power < 0)
   {
      palWritePad(GPIOE, motorPins[num][0], 0);
      palWritePad(GPIOE, motorPins[num][1], 1);
      palSetPad(GPIOE, motorPins[num][2]);
      values[num] = power;
   }
   else
   {
      palWritePad(GPIOE, motorPins[num][0], 0);
      palWritePad(GPIOE, motorPins[num][1], 0);
      palClearPad(GPIOE, motorPins[num][2]);
      values[num] = 100;
   }
   braking[num] = 0;
}

void aiMotorBrake(int num, int power)
{
   palWritePad(GPIOE, motorPins[num][0], 0);
   palWritePad(GPIOE, motorPins[num][1], 0);
   palSetPad(GPIOE, motorPins[num][2]);
   braking[num] = 1;
   values[num] = power;
}

float aiMotorCurrent(int num)
{
   if (num == 0)
      return aiAnalogInput(M0_SENSE)*1.0/585.0;
   else if (num == 1)
      return aiAnalogInput(M1_SENSE)*1.0/585.0;
      
   return -1;
}