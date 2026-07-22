#include "Drivers/gy6500.h"

#define REG_PWR_MGMT_1    0x6B
#define REG_ACCEL_XOUT_H  0x3B

#define ACCEL_SCALE_M_S2  (9.80665f / 16384.0f)
#define GYRO_SCALE_DPS    (1.0f / 131.0f)
#define GRAVITY_M_S2      9.80665f
#define M_PI              3.14159265358979323846f

#define ALPHA                 0.70f
#define ACCEL_NOISE_DEADBAND  0.10f

HAL_StatusTypeDef GY6500_Init(I2C_HandleTypeDef *hi2c) {
	uint8_t pwr_mgmt = 0x00;
	return HAL_I2C_Mem_Write(hi2c, GY6500_I2C_ADDR, REG_PWR_MGMT_1, 1, &pwr_mgmt, 1, 100);
}

HAL_StatusTypeDef GY6500_Calibrate(I2C_HandleTypeDef *hi2c, GY6500_Calib_t *cal, uint16_t samples) {
	float sum_ax = 0, sum_ay = 0, sum_az = 0;
	float sum_gx = 0, sum_gy = 0, sum_gz = 0;
	uint8_t buffer[14];

	if (samples == 0) samples = 500;

	for (uint16_t i = 0; i < samples; i++) {
		if (HAL_I2C_Mem_Read(hi2c, GY6500_I2C_ADDR, REG_ACCEL_XOUT_H, 1, buffer, 14, 100) != HAL_OK) {
			return HAL_ERROR;
		}

		sum_ax += (int16_t)((buffer[0] << 8) | buffer[1]) * ACCEL_SCALE_M_S2;
		sum_ay += (int16_t)((buffer[2] << 8) | buffer[3]) * ACCEL_SCALE_M_S2;
		sum_az += (int16_t)((buffer[4] << 8) | buffer[5]) * ACCEL_SCALE_M_S2;
		sum_gx += (int16_t)((buffer[8] << 8) | buffer[9]) * GYRO_SCALE_DPS;
		sum_gy += (int16_t)((buffer[10] << 8) | buffer[11]) * GYRO_SCALE_DPS;
		sum_gz += (int16_t)((buffer[12] << 8) | buffer[13]) * GYRO_SCALE_DPS;

		HAL_Delay(2);
	}

	cal->ax_offset = sum_ax / samples;
	cal->ay_offset = sum_ay / samples;
	cal->az_offset = (sum_az / samples) - GRAVITY_M_S2;

	cal->gx_offset = sum_gx / samples;
	cal->gy_offset = sum_gy / samples;
	cal->gz_offset = sum_gz / samples;

	return HAL_OK;
}

void GY6500_ResetKinematics(GY6500_Data_t *state) {
	if (!state) return;
	state->vel_x = 0.0f;
	state->vel_y = 0.0f;
	state->vel_z = 0.0f;
	state->pos_x = 0.0f;
	state->pos_y = 0.0f;
	state->pos_z = 0.0f;
}

GY6500_Data_t GY6500_Poll(I2C_HandleTypeDef *hi2c, const GY6500_Calib_t *cal, GY6500_Data_t *prev_state) {
	GY6500_Data_t data = {0.0f};
	uint8_t buffer[14];

	// Preserve accumulated state if passed
	if (prev_state) {
		data = *prev_state;
	}

	// Calculate delta time internally using stored last_tick
	uint32_t current_tick = HAL_GetTick();
	float dt = 0.0f;

	if (data.last_tick > 0) {
		dt = (float)(current_tick - data.last_tick) / 1000.0f;
	}

	// Store current tick for next call
	data.last_tick = current_tick;

	if (HAL_I2C_Mem_Read(hi2c, GY6500_I2C_ADDR, REG_ACCEL_XOUT_H, 1, buffer, 14, 100) == HAL_OK) {
		int16_t raw_ax   = (int16_t)((buffer[0] << 8) | buffer[1]);
		int16_t raw_ay   = (int16_t)((buffer[2] << 8) | buffer[3]);
		int16_t raw_az   = (int16_t)((buffer[4] << 8) | buffer[5]);
		int16_t raw_temp = (int16_t)((buffer[6] << 8) | buffer[7]);
		int16_t raw_gx   = (int16_t)((buffer[8] << 8) | buffer[9]);
		int16_t raw_gy   = (int16_t)((buffer[10] << 8) | buffer[11]);
		int16_t raw_gz   = (int16_t)((buffer[12] << 8) | buffer[13]);

		// Calibrated sensor readouts
		data.accel_x = (raw_ax * ACCEL_SCALE_M_S2) - (cal ? cal->ax_offset : 0.0f);
		data.accel_y = (raw_ay * ACCEL_SCALE_M_S2) - (cal ? cal->ay_offset : 0.0f);
		data.accel_z = (raw_az * ACCEL_SCALE_M_S2) - (cal ? cal->az_offset : 0.0f);

		data.temp    = ((float)raw_temp / 333.87f) + 21.0f;

		data.gyro_x  = (raw_gx * GYRO_SCALE_DPS) - (cal ? cal->gx_offset : 0.0f);
		data.gyro_y  = (raw_gy * GYRO_SCALE_DPS) - (cal ? cal->gy_offset : 0.0f);
		data.gyro_z  = (raw_gz * GYRO_SCALE_DPS) - (cal ? cal->gz_offset : 0.0f);

		// Perform integrations only if dt is valid (ignores the very first call)
		if (dt > 0.0f && dt < 0.2f) {
			float accel_pitch = atan2f(-data.accel_x, sqrtf(data.accel_y * data.accel_y + data.accel_z * data.accel_z)) * (180.0f / M_PI);
			float accel_roll  = atan2f(data.accel_y, data.accel_z) * (180.0f / M_PI);

			data.pitch = ALPHA * (data.pitch + data.gyro_y * dt) + (1.0f - ALPHA) * accel_pitch;
			data.roll  = ALPHA * (data.roll  + data.gyro_x * dt) + (1.0f - ALPHA) * accel_roll;
			data.yaw  += data.gyro_z * dt;

			float pitch_rad = data.pitch * (M_PI / 180.0f);
			float roll_rad  = data.roll  * (M_PI / 180.0f);

			float lin_ax = data.accel_x + (GRAVITY_M_S2 * sinf(pitch_rad));
			float lin_ay = data.accel_y - (GRAVITY_M_S2 * sinf(roll_rad) * cosf(pitch_rad));
			float lin_az = data.accel_z - (GRAVITY_M_S2 * cosf(roll_rad) * cosf(pitch_rad));

			if (fabsf(lin_ax) < ACCEL_NOISE_DEADBAND) lin_ax = 0.0f;
			if (fabsf(lin_ay) < ACCEL_NOISE_DEADBAND) lin_ay = 0.0f;
			if (fabsf(lin_az) < ACCEL_NOISE_DEADBAND) lin_az = 0.0f;

			data.vel_x += lin_ax * dt;
			data.vel_y += lin_ay * dt;
			data.vel_z += lin_az * dt;

			data.pos_x += data.vel_x * dt;
			data.pos_y += data.vel_y * dt;
			data.pos_z += data.vel_z * dt;
		}
	}

	return data;
}

Orientation_t Get_Orientation_Accel(float ax, float ay, float az) {
    Orientation_t ori;

    // Roll: rotation around X axis (-PI to +PI)
    ori.roll_rad = atan2f(ay, az);

    // Pitch: rotation around Y axis (-PI/2 to +PI/2)
    ori.pitch_rad = atan2f(-ax, sqrtf(ay * ay + az * az));

    ori.roll_deg  = ori.roll_rad * (180.0f / M_PI);
    ori.pitch_deg = ori.pitch_rad * (180.0f / M_PI);

    return ori;
}
