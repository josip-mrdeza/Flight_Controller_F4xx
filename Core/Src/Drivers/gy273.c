#include "Drivers/gy273.h"
#include "Drivers/i2c_helper.h"
#include "LCD/ssd1315.h"
#include <stdio.h>

#define REG_CONTROL_1     0x09
#define REG_SET_RESET     0x0B
#define REG_DATA_X_LSB    0x00

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

HAL_StatusTypeDef GY273_Init(I2C_HandleTypeDef *hi2c) {
    HAL_StatusTypeDef status;
    uint8_t set_reset = 0x01;

    status = I2C_Mem_Write_DMA(hi2c, GY273_I2C_ADDR, REG_SET_RESET, 1, &set_reset, 1, 100);
    if (status != HAL_OK) {
        char buff[32];
        snprintf(buff, sizeof(buff), "Init GY273: FAIL");
        SSD1315_Line_2(buff);
        SSD1315_UpdateScreen(hi2c);
        HAL_Delay(1000);
        return status;
    }

    uint8_t config = 0x1D;
    status = I2C_Mem_Write_DMA(hi2c, GY273_I2C_ADDR, REG_CONTROL_1, 1, &config, 1, 100);

    char buff[32];
    snprintf(buff, sizeof(buff), "Init GY273: %s", (status == HAL_OK) ? "OK" : "FAIL");
    SSD1315_Line_2(buff);
    SSD1315_UpdateScreen(hi2c);

    return status;
}

GY273_RawData_t GY273_PollRaw(I2C_HandleTypeDef *hi2c) {
    GY273_RawData_t raw = {0};
    uint8_t buffer[7];

    if (I2C_Mem_Read_DMA(hi2c, GY273_I2C_ADDR, REG_DATA_X_LSB, 1, buffer, 7, 100) == HAL_OK) {
        raw.x = (int16_t)((buffer[1] << 8) | buffer[0]);
        raw.y = (int16_t)((buffer[3] << 8) | buffer[2]);
        raw.z = (int16_t)((buffer[5] << 8) | buffer[4]);
    }

    return raw;
}

HAL_StatusTypeDef GY273_AutoCalibrate(I2C_HandleTypeDef *hi2c, GY273_Calib_t *calib, uint32_t duration_ms) {
    if (!calib) return HAL_ERROR;

    int16_t min_x = 32767,  max_x = -32768;
    int16_t min_y = 32767,  max_y = -32768;
    int16_t min_z = 32767,  max_z = -32768;

    uint32_t start_time = HAL_GetTick();

    while ((HAL_GetTick() - start_time) < duration_ms) {
        GY273_RawData_t raw = GY273_PollRaw(hi2c);

        if (raw.x != 0 || raw.y != 0 || raw.z != 0) {
            if (raw.x < min_x) min_x = raw.x;
            if (raw.x > max_x) max_x = raw.x;

            if (raw.y < min_y) min_y = raw.y;
            if (raw.y > max_y) max_y = raw.y;

            if (raw.z < min_z) min_z = raw.z;
            if (raw.z > max_z) max_z = raw.z;
        }

        HAL_Delay(20);
    }

    calib->x_offset = (float)(max_x + min_x) / 2.0f;
    calib->y_offset = (float)(max_y + min_y) / 2.0f;
    calib->z_offset = (float)(max_z + min_z) / 2.0f;
    calib->is_calibrated = 1;

    return HAL_OK;
}

float GY273_GetHeading2D(const GY273_RawData_t *raw, const GY273_Calib_t *calib) {
    if (!raw) return 0.0f;

    float cx = (float)raw->x - (calib ? calib->x_offset : 0.0f);
    float cy = (float)raw->y - (calib ? calib->y_offset : 0.0f);

    float heading = atan2f(cy, cx) * (180.0f / M_PI);

    if (heading < 0.0f) {
        heading += 360.0f;
    }

    return heading;
}

float GY273_GetHeading3D(const GY273_RawData_t *raw, const GY273_Calib_t *calib, float roll_rad, float pitch_rad) {
    if (!raw) return 0.0f;

    float mx = (float)raw->x - (calib ? calib->x_offset : 0.0f);
    float my = (float)raw->y - (calib ? calib->y_offset : 0.0f);
    float mz = (float)raw->z - (calib ? calib->z_offset : 0.0f);

    float cos_roll  = cosf(roll_rad);
    float sin_roll  = sinf(roll_rad);
    float cos_pitch = cosf(pitch_rad);
    float sin_pitch = sinf(pitch_rad);

    float x_h = mx * cos_pitch + mz * sin_pitch;
    float y_h = mx * sin_roll * sin_pitch + my * cos_roll - mz * sin_roll * cos_pitch;

    float heading = atan2f(y_h, x_h) * (180.0f / M_PI);

    if (heading < 0.0f) {
        heading += 360.0f;
    }

    return heading;
}
