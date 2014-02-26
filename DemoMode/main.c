#include "main.h"

#ifndef DATE
#define DATE "unknown"
#endif
#define debugPrintf(fmt, ...) chprintf((BaseSequentialStream *)&SD2, fmt, ##__VA_ARGS__)

extern Mutex _scrollMtx;
void runTests(void);

void sendResponse(uint8_t command, uint8_t response)
{
   sdPut(_interface, response|command);
}

int getByte(void)
{
   return sdGetTimeout(_interface, DEFAULT_TIMEOUT);
   //uint8_t cmd = 0;
   //sdRead(_interface,&cmd,1);
   //return cmd;
}

void flushInterface(void)
{
   // empty output buffer
   while (sdPutWouldBlock(_interface));

   // very small sleep to reduce chance of race conditions
   chThdSleepMilliseconds(1);

   // empty input buffer
   while (!sdGetWouldBlock(_interface))
      sdGet(_interface);
}

int main(void)
{
   runTests();
   return 0;
}

void sendString(char* s) 
{
   int i=0;

   while (s[i] != 0) {
      sdPut(_interface, s[i]);
      i++;
   }
}

void runTests()
{
   int i=1;
   int j;
   int servoNum=0;
   int servoPos=75;
   int servoDir=1;
   int motorNum=0;
   int motorSpeed=0;
   int motorDir=1;
   uint8_t command[20];

   scrollPause();
   scrollMessage("Press BTN0 for next option, BTN1 to select", 0, 0, 16);
   scrollSetDelay(200);
   scrollEnable();
   chMtxLock(&_scrollMtx);
   lcd_clear();
   chMtxUnlock();

   while (1) {
      // wait for button to be pushed down
      if (button_get(0)) 
      {
         chThdSleepMilliseconds(20);

         //wait for button release
         while (button_get(0)) 
         {
            chThdSleepMilliseconds(1);
         }

         //lcd_clear();
         chMtxLock(&_scrollMtx);
         lcd_cursor(0,1);
         switch (i) 
         {
         case 1:
            lcd_printf("Analog Test  1/6");
            break;
         case 2:
            lcd_printf("Digital Test 2/6");
            break;
         case 3:
            lcd_printf("Servo Test   3/6");
            break;
         case 4:
            lcd_printf("Motor Test   4/6");
            break;
         case 5:
            lcd_printf("Terminal     5/6");
            break;
         case 6:
            lcd_printf("Firmware Date6/6");
            break;
         default:
            break;
         }
         chMtxUnlock();

         i++;
         if (i>6)
            i=1;
      } else if (button_get(1)) 
      {
         scrollPause();
         chThdSleepMilliseconds(20);

         //wait for button release
         while (button_get(1)) 
            chThdSleepMilliseconds(1);

         i--;
         if (i == 0)
            i=6;

         switch (i) 
         {
         case 1:
            //run analog test
            _analog_init();

            while (1) 
            {
               lcd_clear();
               lcd_cursor(0,0);
               for (j=0; j<4; j++)
               {
                  lcd_printf ("%4d", analog_get(j));
               }

               lcd_cursor(0,1);
               for (j=4; j<8; j++)
               {
                  lcd_printf ("%4d", analog_get(j));
               }

               delayMs(100);
            }
            break;
         case 2:
            //run digital test
            lcd_clear();
            lcd_cursor(0,1);
            lcd_printf("0");
            lcd_cursor(7,1);
            lcd_printf("7");
            lcd_cursor(14,1);
            lcd_printf("14");

            while (1) 
            {
               lcd_cursor(0,0);
               for (j=0; j<15; j++)
               {
                  lcd_printf ("%d", digital_get(j));
               }
               delayMs(100);
            }
            break;
         case 3:
            //run servo test
            _servo_init();
            lcd_clear();
            scrollMessage("Press BTN0 to change servo, tap BTN1 for direction, hold BTN1 to change value", 1, 2, 14);
            scrollEnable();

            while (1) 
            {
               chMtxLock(&_scrollMtx);
               lcd_cursor(0,0);
               lcd_printf("Servo: %d", servoNum);
               lcd_cursor(9,0);
               lcd_printf("Pos: %d", servoPos);
               lcd_cursor(0,1);

               if (servoDir) 
               {
                  lcd_printf("+");
               } else {
                  lcd_printf("-");
               }
               chMtxUnlock();

               if (button_get(0)) 
               {
                  chThdSleepMilliseconds(20);

                  //wait for button release
                  while (button_get(0)) 
                     chThdSleepMilliseconds(1);

                  servoNum++;
                  servoPos = 75;
                  if (servoNum > 7)
                     servoNum = 0;
               }

               if (button_get(1)) 
               {
                  chThdSleepMilliseconds(250);
                  if (button_get(1) == 0) //button 1 tap
                  {
                     servoDir = (servoDir ^ 1) & 1; //toggle servo direction
                     chMtxLock(&_scrollMtx);
                     lcd_cursor(0,1);
                     if (servoDir) 
                     {
                        lcd_printf("+");
                     } else {
                        lcd_printf("-");
                     }
                     chMtxUnlock();
                  } else { //button hold
                     while(button_get(1)) 
                     {
                        if (servoDir)
                           servoPos++;
                        else
                           servoPos--;

                        chMtxLock(&_scrollMtx);
                        lcd_cursor(9,0);
                        lcd_printf("Pos: %d\n", servoPos);
                        chMtxUnlock();
                        servo_set(servoNum, servoPos); 
                        chThdSleepMilliseconds(175);
                     }
                  }
               }

               chThdSleepMilliseconds(50);
            }
            break;
         case 4:
            //run motor test
            _motor_init();
            lcd_clear();
            scrollMessage("Press BTN0 to change motor, tap BTN1 for direction, hold BTN1 to change value", 1, 2, 14);
            scrollEnable();

            while (1) 
            {
               chMtxLock(&_scrollMtx);
               lcd_cursor(0,0);
               lcd_printf("Motor: %d", motorNum);
               lcd_cursor(9,0);
               lcd_printf("Spd:%3d", motorSpeed);
               lcd_cursor(0,1);

               if (motorDir) 
               {
                  lcd_printf("+");
               } else {
                  lcd_printf("-");
               }
               chMtxUnlock();

               if (button_get(0)) 
               {
                  chThdSleepMilliseconds(20);

                  //wait for button release
                  while (button_get(0)) 
                     chThdSleepMilliseconds(1);

                  motorNum++;
                  motorSpeed=0;
                  if (motorNum > 1)
                     motorNum = 0;
               }

               if (button_get(1)) 
               {
                  chThdSleepMilliseconds(250);
                  if (button_get(1) == 0) //button 1 tap
                  {
                     motorDir = (motorDir ^ 1) & 1; //toggle servo direction
                     chMtxLock(&_scrollMtx);
                     lcd_cursor(0,1);
                     if (motorDir) 
                     {
                        lcd_printf("+");
                     } else {
                        lcd_printf("-");
                     }
                     chMtxUnlock();
                  } else { //button hold
                     while(button_get(1)) 
                     {
                        if (motorDir) 
                        {
                           motorSpeed++;
                           if (motorSpeed > 100)
                              motorSpeed = 100;
                        } else {
                           motorSpeed--;
                           if (motorSpeed < -100)
                              motorSpeed = -100;
                        }

                        chMtxLock(&_scrollMtx);
                        lcd_cursor(9,0);
                        lcd_printf("Spd:%3d\n", motorSpeed);
                        chMtxUnlock();
                        motor_set(motorNum, motorSpeed); 
                        chThdSleepMilliseconds(75);
                     }
                  }
               }

               chThdSleepMilliseconds(50);
            }
            break;
         case 5:
            //terminal mode
            _interface = _interfaces[2]; //use the USB interface
            _motor_init();
            _servo_init();
            _analog_init();

            lcd_clear();
            scrollMessage("Terminal Mode - 38400 Baud 8-N-1", 0, 0, 16);
            scrollEnable();
            sendString("\n\r\n\rWelcome to the Aithon Board\n\r");
            sendString("(commands will not be echoed to terminal)\n\r");
            sendString("send 'h' for help message\n\r");

            while (1) 
            {
               //read character until a carriage return (13)
               i=-1;
               do {
                  i++;
                  sdRead(_interface,&(command[i]),1);
               } while (command[i] != 13);
               command[i]=0;

               if (command[0]=='h' || i==0)  //if help command
               { 
                  sendString("Welcome to the Aithon Board\n\r");
                  sendString("(commands will not be echoed to terminal)\n\r");
                  sendString("mxdyyy (x=motor number, d=-/+ direction, yyy=3-digit speed)\n\r");
                  sendString("sxyyy (x=servo number, yyy=3-digit position)\n\r");
                  sendString("ax (x=analog input)\n\r");
                  sendString("dx (x=digital input)\n\r");
                  sendString("bx (x=button number)\n\r");
               } else if (command[0] == 'm') {
                  //motor command
                  int k;
                  int motor_speed=0;

                  for(k=0; k<2; k++)
                  {
                     motor_speed += command[k+3] - 48;
                     motor_speed *= 10;
                  }
                  motor_speed += command[k+3] - 48;

                  if (command[2] == '-')
                     motor_speed = -motor_speed;

                  motor_set(command[1]-48, motor_speed);
               } else if (command[0] == 's') {
                  //servo command
                  int k;
                  int servo_pos=0;

                  for(k=0; k<2; k++)
                  {
                     servo_pos += command[k+2] - 48;
                     servo_pos *= 10;
                  }
                  servo_pos += command[k+2] - 48;

                  servo_set(command[1]-48, servo_pos);
               } else if (command[0] == 'a') {
                  //read analog command
                  int pin=0;
                  int value;
                  int k;
                  int divider=1000;

                  pin = command[1] - 48;
                  value = analog_get(pin);
                  
                  for (k=0; k<4; k++) {
                     sdPut(_interface, (value/divider) + 48);
                     value = value % divider;
                     divider = divider / 10;
                  }
                  sdPut(_interface, 10);
                  sdPut(_interface, 13);
                } else {
                  //for testing just echo back the command
                  i=0;
                  while(command[i] != 0) {
                     sdPut(_interface, command[i]);
                     i++;
                  } 
                  sdPut(_interface, 10);
                  sdPut(_interface, 13);
               }
               chThdSleepMilliseconds(1);
            }

            break;
         case 6:
            //bootloader date
            lcd_clear();
            lcd_printf(DATE);
            while(1) 
            {
               chThdSleepMilliseconds(1);
            }
            break;
         default:
            break;
         }
      }

      chThdSleepMilliseconds(1);
   }

   return;
}
