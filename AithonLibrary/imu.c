#include "Aithon.h"

#if AI_USE_IMU || defined(__DOXYGEN__)

#define ACCEL_ADDR   0b00011001
#define GYRO_ADDR    0b01101010
#define CTRL_REG1    0x20
#define CTRL_REG4    0x23
#define OUT_X_L      0x28
#define OUT_X_H      0x29
#define OUT_Y_L      0x2A
#define OUT_Y_H      0x2B
#define OUT_Z_L      0x2C
#define OUT_Z_H      0x2D
#define OUT_TEMP_G   0x26

void imuWriteReg(IMUDevice dev, uint8_t addr, uint8_t data);
uint8_t imuReadReg(IMUDevice dev, uint8_t addr);


static const I2CConfig imu_config = {
   OPMODE_I2C,
   100000,
   STD_DUTY_CYCLE,
};

/**
 * @brief   IMU function initialization.
 * @details This function sets up and initializes the on-board 3-axis gyroscope
 *          and accelerometer.
 * @note    This function is called by aiInit() and does not need to be called
 *          again by the user after aiInit() is called.
 */
void aiIMUInit(void)
{
   i2cInit();
   i2cStart(&I2CD1, &imu_config);
   imuWriteReg(ACCEL, CTRL_REG1, 0x77); // normal mode, all axes enabled, 400Hz
   imuWriteReg(ACCEL, CTRL_REG4, 0x08); // high resolution output mode
   imuWriteReg(GYRO, CTRL_REG1, 0x8F); // normal mode, all axes enabled, 380Hz
}

void imuWriteReg(IMUDevice dev, uint8_t addr, uint8_t data)
{
   uint8_t tx_buf[2] = {addr, data};
   if (dev == ACCEL)
   {
      i2cMasterTransmit(&I2CD1, ACCEL_ADDR, tx_buf, 2, NULL, 0);
   }
   else if (dev == GYRO)
   {
      i2cMasterTransmit(&I2CD1, GYRO_ADDR, tx_buf, 2, NULL, 0);
   }
}

uint8_t imuReadReg(IMUDevice dev, uint8_t addr)
{
   uint8_t data;
   if (dev == ACCEL)
   {
      i2cMasterTransmit(&I2CD1, ACCEL_ADDR, &addr, 1, &data, 1);
   }
   else if (dev == GYRO)
   {
      i2cMasterTransmit(&I2CD1, GYRO_ADDR, &addr, 1, &data, 1);
   }
   return data;
}

IMUReading aiIMUGetReading(IMUDevice dev)
{
   IMUReading reading;
   reading.x = (int16_t) (((uint16_t)imuReadReg(dev, OUT_X_H) << 8) | imuReadReg(dev, OUT_X_L));
   reading.y = (int16_t) (((uint16_t)imuReadReg(dev, OUT_Y_H) << 8) | imuReadReg(dev, OUT_Y_L));
   reading.z = (int16_t) (((uint16_t)imuReadReg(dev, OUT_Z_H) << 8) | imuReadReg(dev, OUT_Z_L));
   return reading;
}

int8_t aiIMUGetTemp(void)
{
   return (int8_t) imuReadReg(GYRO, OUT_TEMP_G);
}

#endif