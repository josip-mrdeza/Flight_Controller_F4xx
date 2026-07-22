#ifndef DRIVERS_GY6500_GY6500_H_
#define DRIVERS_GY6500_GY6500_H_

#include "stm32f4xx_hal.h"
#include <math.h>

#define GY6500_I2C_ADDR         (0x68 << 1)

typedef struct {
    // Raw calibrated sensor readouts
    float accel_x; // m/s^2
    float accel_y; // m/s^2
    float accel_z; // m/s^2
    float temp;    // °C
    float gyro_x;  // deg/s
    float gyro_y;  // deg/s
    float gyro_z;  // deg/s

    // Attitude State (Rotation relative to world frame)
    float roll;    // degrees (rotation around X)
    float pitch;   // degrees (rotation around Y)
    float yaw;     // degrees (integrated Z rotation)

    // Kinematic State (World/Body frame movement)
    float vel_x;   // m/s
    float vel_y;   // m/s
    float vel_z;   // m/s
    float pos_x;   // m
    float pos_y;   // m
    float pos_z;   // m

    // Timestamp tracking for internal delta-time calculation
    uint32_t last_tick;
} GY6500_Data_t;

typedef struct {
    float ax_offset;
    float ay_offset;
    float az_offset;
    float gx_offset;
    float gy_offset;
    float gz_offset;
} GY6500_Calib_t;

typedef struct {
    float roll_rad;   // Rotation around X-axis
    float pitch_rad;  // Rotation around Y-axis
    float roll_deg;
    float pitch_deg;
} Orientation_t;

HAL_StatusTypeDef GY6500_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef GY6500_Calibrate(I2C_HandleTypeDef *hi2c, GY6500_Calib_t *cal, uint16_t samples);

void GY6500_ResetKinematics(GY6500_Data_t *state);

// Polls IMU, updates state, and handles internal dt calculation via state->last_tick
GY6500_Data_t GY6500_Poll(I2C_HandleTypeDef *hi2c, const GY6500_Calib_t *cal, GY6500_Data_t *prev_state);
Orientation_t Get_Orientation_Accel(float ax, float ay, float az);
#endif /* DRIVERS_GY6500_GY6500_H_ */
