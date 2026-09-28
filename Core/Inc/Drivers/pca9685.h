#ifndef DRIVERS_PCA9685_H_
#define DRIVERS_PCA9685_H_

#include "stm32f4xx_hal.h"
#include <math.h>

#define PCA9685_I2C_ADDR        (0x40 << 1)

#define PCA9685_MODE1           0x00
#define PCA9685_MODE2           0x01
#define PCA9685_PRESCALE        0xFE
#define PCA9685_LED0_ON_L       0x06

#define PCA9685_OSC_FREQ        25000000.0f

extern uint8_t g_pca9685_addr;

HAL_StatusTypeDef PCA9685_Init(I2C_HandleTypeDef *hi2c, float freq_hz);
HAL_StatusTypeDef PCA9685_SetPWMFrequency(I2C_HandleTypeDef *hi2c, float freq_hz);
HAL_StatusTypeDef PCA9685_SetPWM(I2C_HandleTypeDef *hi2c, uint8_t channel, uint16_t on_count, uint16_t off_count);
HAL_StatusTypeDef PCA9685_SetDutyCycle(I2C_HandleTypeDef *hi2c, uint8_t channel, float duty_percent);
HAL_StatusTypeDef PCA9685_SetServoPulse(I2C_HandleTypeDef *hi2c, uint8_t channel, uint16_t pulse_us, float freq_hz);

#endif
