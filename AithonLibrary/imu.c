#include "Aithon.h"

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


static const I2CConfig imu_config = {
   OPMODE_I2C,
   100000,
   STD_DUTY_CYCLE,
};

void _imu_write(uint8_t device_addr, uint8_t addr, uint8_t data)
{
   uint8_t tx_buf[2] = {addr, data};
   i2cMasterTransmit(&I2CD1, device_addr, tx_buf, 2, NULL, 0);
}

uint8_t _imu_read(uint8_t device_addr, uint8_t addr)
{
   uint8_t data = 0;
   i2cMasterTransmit(&I2CD1, device_addr, &addr, 1, &data, 1);
   return data;
}

/**
 * @brief   IMU function initialization.
 * @details This function sets up and initializes the on-board 3-axis gyroscope
 *          and accelerometer.
 * @note    This function is called by aiInit() and does not need to be called
 *          again by the user after aiInit() is called.
 */
void _imu_init(void)
{
   i2cInit();
   i2cStart(&I2CD1, &imu_config);
   _imu_write(ACCEL, CTRL_REG1, 0x77); // normal mode, all axes enabled, 400Hz
   _imu_write(ACCEL, CTRL_REG4, 0x08); // high resolution output mode
   _imu_write(GYRO, CTRL_REG1, 0x8F); // normal mode, all axes enabled, 380Hz
}

IMUReading imu_getReading(IMUDevice device)
{
   IMUReading reading = {0, 0, 0};
   uint8_t device_addr = 0;
   if (device == ACCEL)
      device_addr = ACCEL_ADDR;
   else if (device == GYRO)
      device_addr = GYRO_ADDR;
   else
      return reading;
   
   reading.x = (int16_t) (((uint16_t)_imu_read(device_addr, OUT_X_H) << 8) | _imu_read(device_addr, OUT_X_L));
   reading.y = (int16_t) (((uint16_t)_imu_read(device_addr, OUT_Y_H) << 8) | _imu_read(device_addr, OUT_Y_L));
   reading.z = (int16_t) (((uint16_t)_imu_read(device_addr, OUT_Z_H) << 8) | _imu_read(device_addr, OUT_Z_L));
   return reading;
}

int8_t imu_getTemp(void)
{
   return (int8_t) _imu_read(GYRO_ADDR, OUT_TEMP_G);
}
