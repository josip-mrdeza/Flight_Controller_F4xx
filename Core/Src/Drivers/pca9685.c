#include "Drivers/pca9685.h"

HAL_StatusTypeDef PCA9685_Init(I2C_HandleTypeDef *hi2c, float freq_hz) {
    HAL_StatusTypeDef status;

    // Reset MODE1 register (Auto-Increment enabled: 0x20)
    uint8_t mode1 = 0x20;
    status = HAL_I2C_Mem_Write(hi2c, PCA9685_I2C_ADDR, PCA9685_MODE1, 1, &mode1, 1, 100);
    if (status != HAL_OK) return status;

    // Set output PWM frequency
    return PCA9685_SetPWMFrequency(hi2c, freq_hz);
}

HAL_StatusTypeDef PCA9685_SetPWMFrequency(I2C_HandleTypeDef *hi2c, float freq_hz) {
    HAL_StatusTypeDef status;

    // Clamp frequency limits
    if (freq_hz < 24.0f) freq_hz = 24.0f;
    if (freq_hz > 1526.0f) freq_hz = 1526.0f;

    // Prescale formula from datasheet: prescale = round(osc_clock / (4096 * freq)) - 1
    uint8_t prescale = (uint8_t)(floorf(PCA9685_OSC_FREQ / (4096.0f * freq_hz) + 0.5f) - 1.0f);

    // Read current MODE1 register
    uint8_t old_mode = 0x00;
    status = HAL_I2C_Mem_Read(hi2c, PCA9685_I2C_ADDR, PCA9685_MODE1, 1, &old_mode, 1, 100);
    if (status != HAL_OK) return status;

    // To set PRESCALE, bit 4 (SLEEP) of MODE1 must be set to 1
    uint8_t sleep_mode = (old_mode & 0x7F) | 0x10; // SLEEP = 1
    uint8_t wake_mode  = (old_mode & 0x7F) & ~0x10; // SLEEP = 0

    HAL_I2C_Mem_Write(hi2c, PCA9685_I2C_ADDR, PCA9685_MODE1, 1, &sleep_mode, 1, 100);
    HAL_I2C_Mem_Write(hi2c, PCA9685_I2C_ADDR, PCA9685_PRESCALE, 1, &prescale, 1, 100);
    HAL_I2C_Mem_Write(hi2c, PCA9685_I2C_ADDR, PCA9685_MODE1, 1, &wake_mode, 1, 100);

    HAL_Delay(5); // Wait for oscillator to stabilize

    // Enable Auto-Increment (0x20) and restart (0x80)
    uint8_t restart_mode = wake_mode | 0xa0; // AUTO-INC | RESTART
    return HAL_I2C_Mem_Write(hi2c, PCA9685_I2C_ADDR, PCA9685_MODE1, 1, &restart_mode, 1, 100);
}

HAL_StatusTypeDef PCA9685_SetPWM(I2C_HandleTypeDef *hi2c, uint8_t channel, uint16_t on_count, uint16_t off_count) {
    if (channel > 15) return HAL_ERROR;

    uint8_t reg_addr = PCA9685_LED0_ON_L + (4 * channel);
    uint8_t buffer[4];

    buffer[0] = (uint8_t)(on_count & 0xFF);
    buffer[1] = (uint8_t)(on_count >> 8);
    buffer[2] = (uint8_t)(off_count & 0xFF);
    buffer[3] = (uint8_t)(off_count >> 8);

    // Write all 4 registers at once using Auto-Increment
    return HAL_I2C_Mem_Write(hi2c, PCA9685_I2C_ADDR, reg_addr, 1, buffer, 4, 100);
}

HAL_StatusTypeDef PCA9685_SetDutyCycle(I2C_HandleTypeDef *hi2c, uint8_t channel, float duty_percent) {
    if (duty_percent < 0.0f) duty_percent = 0.0f;
    if (duty_percent > 100.0f) duty_percent = 100.0f;

    uint16_t off_count = (uint16_t)((duty_percent / 100.0f) * 4095.0f);

    // Fully ON mode if 100%
    if (off_count >= 4095) {
        return PCA9685_SetPWM(hi2c, channel, 4096, 0);
    }
    // Fully OFF mode if 0%
    if (off_count == 0) {
        return PCA9685_SetPWM(hi2c, channel, 0, 0);
    }

    return PCA9685_SetPWM(hi2c, channel, 0, off_count);
}

HAL_StatusTypeDef PCA9685_SetServoPulse(I2C_HandleTypeDef *hi2c, uint8_t channel, uint16_t pulse_us, float freq_hz) {
    // Period in microseconds = (1 / freq) * 1,000,000
    float period_us = 1000000.0f / freq_hz;

    // Convert pulse width in us to 12-bit count (0 - 4095)
    uint16_t off_count = (uint16_t)((pulse_us / period_us) * 4095.0f);

    return PCA9685_SetPWM(hi2c, channel, 0, off_count);
}
