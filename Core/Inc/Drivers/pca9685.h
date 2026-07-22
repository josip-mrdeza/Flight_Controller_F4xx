#ifndef DRIVERS_PCA9685_H_
#define DRIVERS_PCA9685_H_

#include "stm32f4xx_hal.h"
#include <math.h>

// Default PCA9685 I2C Address (0x40 shifted left for STM32 HAL -> 0x80)
#define PCA9685_I2C_ADDR        (0x40 << 1)

// PCA9685 Registers
#define PCA9685_MODE1           0x00
#define PCA9685_PRESCALE        0xFE
#define PCA9685_LED0_ON_L       0x06

// Internal Oscillator Frequency (25 MHz nominal)
#define PCA9685_OSC_FREQ        25000000.0f

HAL_StatusTypeDef PCA9685_Init(I2C_HandleTypeDef *hi2c, float freq_hz);
HAL_StatusTypeDef PCA9685_SetPWMFrequency(I2C_HandleTypeDef *hi2c, float freq_hz);

// Set raw 12-bit ON/OFF counts (0 - 4095) for a specific channel (0 - 15)
HAL_StatusTypeDef PCA9685_SetPWM(I2C_HandleTypeDef *hi2c, uint8_t channel, uint16_t on_count, uint16_t off_count);

// Set duty cycle percentage (0.0% - 100.0%)
HAL_StatusTypeDef PCA9685_SetDutyCycle(I2C_HandleTypeDef *hi2c, uint8_t channel, float duty_percent);

// Set pulse width in microseconds (ideal for RC servos, e.g., 1000us - 2000us at 50Hz)
HAL_StatusTypeDef PCA9685_SetServoPulse(I2C_HandleTypeDef *hi2c, uint8_t channel, uint16_t pulse_us, float freq_hz);

#endif /* DRIVERS_PCA9685_H_ */
