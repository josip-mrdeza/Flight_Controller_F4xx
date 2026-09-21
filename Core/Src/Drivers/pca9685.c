#include "Drivers/pca9685.h"
#include "Drivers/i2c_helper.h"
#include "LCD/ssd1315.h"
#include <stdio.h>

HAL_StatusTypeDef PCA9685_Init(I2C_HandleTypeDef *hi2c, float freq_hz) {
    HAL_StatusTypeDef status;
    uint8_t mode1 = 0x20;
    status = I2C_Mem_Write_DMA(hi2c, PCA9685_I2C_ADDR, PCA9685_MODE1, 1, &mode1, 1, 100);
    if (status != HAL_OK) {
        char buff[32];
        snprintf(buff, sizeof(buff), "Init pca9685: Fail");
        SSD1315_Line_3(buff);
        SSD1315_UpdateScreen(hi2c);
        HAL_Delay(1000);
        return status;
    }

    status = PCA9685_SetPWMFrequency(hi2c, freq_hz);
    char buff[32];
    snprintf(buff, sizeof(buff), "Init pca9685: %s", (status == HAL_OK) ? "Ok" : "Fail");
    SSD1315_Line_3(buff);
    SSD1315_UpdateScreen(hi2c);
    HAL_Delay((status == HAL_OK) ? 50 : 1000);
    return status;
}

HAL_StatusTypeDef PCA9685_SetPWMFrequency(I2C_HandleTypeDef *hi2c, float freq_hz) {
    HAL_StatusTypeDef status;

    if (freq_hz < 24.0f) freq_hz = 24.0f;
    if (freq_hz > 1526.0f) freq_hz = 1526.0f;

    uint8_t prescale = (uint8_t)(floorf(PCA9685_OSC_FREQ / (4096.0f * freq_hz) + 0.5f) - 1.0f);

    uint8_t old_mode = 0x00;
    status = I2C_Mem_Read_DMA(hi2c, PCA9685_I2C_ADDR, PCA9685_MODE1, 1, &old_mode, 1, 100);
    if (status != HAL_OK) return status;

    uint8_t sleep_mode = (old_mode & 0x7F) | 0x10;
    uint8_t wake_mode  = (old_mode & 0x7F) & ~0x10;

    I2C_Mem_Write_DMA(hi2c, PCA9685_I2C_ADDR, PCA9685_MODE1, 1, &sleep_mode, 1, 100);
    I2C_Mem_Write_DMA(hi2c, PCA9685_I2C_ADDR, PCA9685_PRESCALE, 1, &prescale, 1, 100);
    I2C_Mem_Write_DMA(hi2c, PCA9685_I2C_ADDR, PCA9685_MODE1, 1, &wake_mode, 1, 100);

    HAL_Delay(5);

    uint8_t restart_mode = wake_mode | 0xa0;
    return I2C_Mem_Write_DMA(hi2c, PCA9685_I2C_ADDR, PCA9685_MODE1, 1, &restart_mode, 1, 100);
}

HAL_StatusTypeDef PCA9685_SetPWM(I2C_HandleTypeDef *hi2c, uint8_t channel, uint16_t on_count, uint16_t off_count) {
    if (channel > 15) return HAL_ERROR;

    uint8_t reg_addr = PCA9685_LED0_ON_L + (4 * channel);
    uint8_t buffer[4];

    buffer[0] = (uint8_t)(on_count & 0xFF);
    buffer[1] = (uint8_t)(on_count >> 8);
    buffer[2] = (uint8_t)(off_count & 0xFF);
    buffer[3] = (uint8_t)(off_count >> 8);

    return I2C_Mem_Write_DMA(hi2c, PCA9685_I2C_ADDR, reg_addr, 1, buffer, 4, 100);
}

HAL_StatusTypeDef PCA9685_SetDutyCycle(I2C_HandleTypeDef *hi2c, uint8_t channel, float duty_percent) {
    if (duty_percent < 0.0f) duty_percent = 0.0f;
    if (duty_percent > 100.0f) duty_percent = 100.0f;

    uint16_t off_count = (uint16_t)((duty_percent / 100.0f) * 4095.0f);

    if (off_count >= 4095) {
        return PCA9685_SetPWM(hi2c, channel, 4096, 0);
    }
    if (off_count == 0) {
        return PCA9685_SetPWM(hi2c, channel, 0, 0);
    }

    return PCA9685_SetPWM(hi2c, channel, 0, off_count);
}

HAL_StatusTypeDef PCA9685_SetServoPulse(I2C_HandleTypeDef *hi2c, uint8_t channel, uint16_t pulse_us, float freq_hz) {
    float period_us = 1000000.0f / freq_hz;
    uint16_t off_count = (uint16_t)((pulse_us / period_us) * 4095.0f);

    return PCA9685_SetPWM(hi2c, channel, 0, off_count);
}
