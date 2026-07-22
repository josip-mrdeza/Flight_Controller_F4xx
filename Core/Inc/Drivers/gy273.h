#ifndef DRIVERS_GY273_H_
#define DRIVERS_GY273_H_

#include "stm32f4xx_hal.h"
#include <math.h>

#define GY273_I2C_ADDR          (0x0D << 1) // QMC5883L 7-bit 0x0D -> 0x1A

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} GY273_RawData_t;

typedef struct {
    float x_offset;
    float y_offset;
    float z_offset;
    uint8_t is_calibrated;
} GY273_Calib_t;

HAL_StatusTypeDef GY273_Init(I2C_HandleTypeDef *hi2c);
GY273_RawData_t GY273_PollRaw(I2C_HandleTypeDef *hi2c);

// Auto-calibration: Spin sensor in 3D for `duration_ms` (e.g., 10000 ms)
HAL_StatusTypeDef GY273_AutoCalibrate(I2C_HandleTypeDef *hi2c, GY273_Calib_t *calib, uint32_t duration_ms);

// 2D Planar Heading (Flat surface)
float GY273_GetHeading2D(const GY273_RawData_t *raw, const GY273_Calib_t *calib);

// 3D Tilt-Compensated Heading (Supply roll and pitch from accelerometer in radians)
float GY273_GetHeading3D(const GY273_RawData_t *raw, const GY273_Calib_t *calib, float roll_rad, float pitch_rad);

#endif /* DRIVERS_GY273_H_ */
