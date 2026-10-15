/**
  ******************************************************************************
  * @file           : lora_telemetry.c
  * @brief          : Telemetry data structuring, validation, and optimization
  *                   for LoRa packet transmission and reception.
  ******************************************************************************
  */

#include "Drivers/DX-LR30_Driver/lora_telemetry.h"
#include "Drivers/DX-LR30_Driver/adr.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

bool Lora_IsValidFloat(float val)
{
    if (isnan(val) || isinf(val)) {
        return false;
    }
    /* Value is -1.0f (or extremely close) denoting invalid/uninitialized */
    if (fabsf(val - (-1.0f)) < 0.0001f) {
        return false;
    }
    return true;
}

bool Lora_IsValidGps(const GPS_HandleTypeDef *gps)
{
    if (!gps) {
        return false;
    }
    if (!gps->data.is_valid) {
        return false;
    }
    if (!Lora_IsValidFloat(gps->data.latitude) ||
        !Lora_IsValidFloat(gps->data.longitude) ||
        !Lora_IsValidFloat(gps->data.altitude)) {
        return false;
    }
    /* Reject hardcoded default/placeholder coordinates */
    if (fabsf(gps->data.latitude - LORA_DEFAULT_GPS_LAT) < 0.00001f &&
        fabsf(gps->data.longitude - LORA_DEFAULT_GPS_LON) < 0.00001f) {
        return false;
    }
    /* Bounds checking */
    if (gps->data.latitude < -90.0f || gps->data.latitude > 90.0f ||
        gps->data.longitude < -180.0f || gps->data.longitude > 180.0f) {
        return false;
    }
    return true;
}

uint16_t Lora_AppendGps(char *buffer, size_t max_len, const GPS_HandleTypeDef *gps)
{
    if (!buffer || max_len == 0 || !Lora_IsValidGps(gps)) {
        return 0;
    }
    size_t cur_len = strlen(buffer);
    if (cur_len >= max_len - 1) {
        return 0;
    }

    char temp[80];
    if (gps->data.satellites > 0 && gps->data.satellites != 255) {
        snprintf(temp, sizeof(temp), "lt%.6f;lo%.6f;al%.1f;sa%u;",
                 gps->data.latitude, gps->data.longitude, gps->data.altitude,
                 (unsigned int)gps->data.satellites);
    } else {
        snprintf(temp, sizeof(temp), "lt%.6f;lo%.6f;al%.1f;",
                 gps->data.latitude, gps->data.longitude, gps->data.altitude);
    }

    strncat(buffer, temp, max_len - cur_len - 1);
    return (uint16_t)strlen(temp);
}

uint16_t Lora_AppendControllerGps(char *buffer, size_t max_len, const GPS_HandleTypeDef *gps)
{
    if (!buffer || max_len == 0 || !Lora_IsValidGps(gps)) {
        return 0;
    }
    size_t cur_len = strlen(buffer);
    if (cur_len >= max_len - 1) {
        return 0;
    }

    char temp[64];
    snprintf(temp, sizeof(temp), "clt%.6f;clo%.6f;cal%.1f;",
             gps->data.latitude, gps->data.longitude, gps->data.altitude);

    strncat(buffer, temp, max_len - cur_len - 1);
    return (uint16_t)strlen(temp);
}

uint16_t Lora_AppendVelocities(char *buffer, size_t max_len, float vx, float vy, float vz)
{
    if (!buffer || max_len == 0) {
        return 0;
    }
    if (!Lora_IsValidFloat(vx) || !Lora_IsValidFloat(vy) || !Lora_IsValidFloat(vz)) {
        return 0;
    }
    size_t cur_len = strlen(buffer);
    if (cur_len >= max_len - 1) {
        return 0;
    }

    char temp[48];
    snprintf(temp, sizeof(temp), "vx%.2f;vy%.2f;vz%.2f;", vx, vy, vz);
    strncat(buffer, temp, max_len - cur_len - 1);
    return (uint16_t)strlen(temp);
}

uint16_t Lora_AppendAccelerations(char *buffer, size_t max_len, float ax, float ay, float az)
{
    if (!buffer || max_len == 0) {
        return 0;
    }
    if (!Lora_IsValidFloat(ax) || !Lora_IsValidFloat(ay) || !Lora_IsValidFloat(az)) {
        return 0;
    }
    size_t cur_len = strlen(buffer);
    if (cur_len >= max_len - 1) {
        return 0;
    }

    char temp[48];
    snprintf(temp, sizeof(temp), "ax%.2f;ay%.2f;az%.2f;", ax, ay, az);
    strncat(buffer, temp, max_len - cur_len - 1);
    return (uint16_t)strlen(temp);
}

uint16_t Lora_AppendAttitude(char *buffer, size_t max_len, float gx, float gy, float gz, float hd)
{
    if (!buffer || max_len == 0) {
        return 0;
    }
    size_t cur_len = strlen(buffer);
    if (cur_len >= max_len - 1) {
        return 0;
    }

    char temp[64];
    temp[0] = '\0';

    if (Lora_IsValidFloat(gx) && Lora_IsValidFloat(gy) && Lora_IsValidFloat(gz)) {
        snprintf(temp, sizeof(temp), "gx%.2f;gy%.2f;gz%.2f;", gx, gy, gz);
    }

    if (Lora_IsValidFloat(hd)) {
        char hd_str[24];
        snprintf(hd_str, sizeof(hd_str), "hd%.2f;", hd);
        strncat(temp, hd_str, sizeof(temp) - strlen(temp) - 1);
    }

    if (temp[0] != '\0') {
        strncat(buffer, temp, max_len - cur_len - 1);
        return (uint16_t)strlen(temp);
    }
    return 0;
}

uint16_t Lora_AppendEnvironment(char *buffer, size_t max_len, float temp, float pressure)
{
    if (!buffer || max_len == 0) {
        return 0;
    }
    size_t cur_len = strlen(buffer);
    if (cur_len >= max_len - 1) {
        return 0;
    }

    char buf_tmp[48];
    buf_tmp[0] = '\0';

    /* Temperature: reject if 0.0f, -1.0f, or invalid */
    if (Lora_IsValidFloat(temp) && fabsf(temp) > 0.001f) {
        char t_str[24];
        snprintf(t_str, sizeof(t_str), "tm%.1f;", temp);
        strncat(buf_tmp, t_str, sizeof(buf_tmp) - strlen(buf_tmp) - 1);
    }

    /* Pressure: reject if -1.0f, <= 0.0f, or invalid */
    if (Lora_IsValidFloat(pressure) && pressure > 0.0f) {
        char p_str[24];
        snprintf(p_str, sizeof(p_str), "pr%.2f;", pressure);
        strncat(buf_tmp, p_str, sizeof(buf_tmp) - strlen(buf_tmp) - 1);
    }

    if (buf_tmp[0] != '\0') {
        strncat(buffer, buf_tmp, max_len - cur_len - 1);
        return (uint16_t)strlen(buf_tmp);
    }
    return 0;
}

uint16_t Lora_AppendPower(char *buffer, size_t max_len, float v_bat, float i_bat, float v_servo, float i_servo)
{
    if (!buffer || max_len == 0) {
        return 0;
    }
    size_t cur_len = strlen(buffer);
    if (cur_len >= max_len - 1) {
        return 0;
    }

    char buf_tmp[64];
    buf_tmp[0] = '\0';

    if (Lora_IsValidFloat(v_bat) && v_bat > 0.0f) {
        char str[20];
        snprintf(str, sizeof(str), "vb%.2f;", v_bat);
        strncat(buf_tmp, str, sizeof(buf_tmp) - strlen(buf_tmp) - 1);
    }
    if (Lora_IsValidFloat(i_bat) && i_bat >= 0.0f) {
        char str[20];
        snprintf(str, sizeof(str), "ib%.2f;", i_bat);
        strncat(buf_tmp, str, sizeof(buf_tmp) - strlen(buf_tmp) - 1);
    }
    if (Lora_IsValidFloat(v_servo) && v_servo > 0.0f) {
        char str[20];
        snprintf(str, sizeof(str), "vs%.2f;", v_servo);
        strncat(buf_tmp, str, sizeof(buf_tmp) - strlen(buf_tmp) - 1);
    }
    if (Lora_IsValidFloat(i_servo) && i_servo >= 0.0f) {
        char str[20];
        snprintf(str, sizeof(str), "is%.2f;", i_servo);
        strncat(buf_tmp, str, sizeof(buf_tmp) - strlen(buf_tmp) - 1);
    }

    if (buf_tmp[0] != '\0') {
        strncat(buffer, buf_tmp, max_len - cur_len - 1);
        return (uint16_t)strlen(buf_tmp);
    }
    return 0;
}

uint16_t Lora_StructurePlanePacket(char *buffer, size_t max_len, uint8_t tier, const AppContext_t *app)
{
    if (!buffer || max_len < 32 || !app) {
        return 0;
    }
    buffer[0] = '\0';
    snprintf(buffer, max_len, "idRaptorx0;dr%u;", (unsigned int)tier);

    /* 1. Append GPS coordinates (rejected if missing fix, default, or -1) */
    Lora_AppendGps(buffer, max_len, &app->gps);

    /* 2. Append Kinematics (rejected if invalid or -1) */
    Lora_AppendVelocities(buffer, max_len, app->imu_data.vel_x, app->imu_data.vel_y, app->imu_data.vel_z);
    Lora_AppendAccelerations(buffer, max_len, app->imu_data.accel_x, app->imu_data.accel_y, app->imu_data.accel_z);

    /* 3. Append Attitude & Heading (rejected if invalid or -1) */
    float roll = (app->orientation.roll_deg != 0.0f) ? app->orientation.roll_deg : app->imu_data.roll;
    float pitch = (app->orientation.pitch_deg != 0.0f) ? app->orientation.pitch_deg : app->imu_data.pitch;
    float yaw = app->imu_data.yaw;
    float hd = (app->heading_3d != 0.0f) ? app->heading_3d : app->heading_2d;
    Lora_AppendAttitude(buffer, max_len, roll, pitch, yaw, hd);

    /* 4. Append Environment (temp/pressure rejected if default/0.0/-1/invalid) */
    float temp = (app->imu_data.temp != 0.0f) ? app->imu_data.temp : -1.0f;
    Lora_AppendEnvironment(buffer, max_len, temp, -1.0f);

    /* 5. Append Power (battery/servo metrics rejected if unmeasured/-1) */
    Lora_AppendPower(buffer, max_len, -1.0f, -1.0f, -1.0f, -1.0f);

    /* 6. Append Line Terminator */
    size_t cur_len = strlen(buffer);
    if (cur_len < max_len - 2) {
        strncat(buffer, "\r\n", max_len - cur_len - 1);
    }
    return (uint16_t)strlen(buffer);
}

uint16_t Lora_StructureControllerPacket(char *buffer, size_t max_len, uint8_t tier, const AppContext_t *app)
{
    if (!buffer || max_len < 16) {
        return 0;
    }
    buffer[0] = '\0';
    snprintf(buffer, max_len, "ctrl;dr%u;", (unsigned int)tier);

    if (app) {
        /* Append controller GPS only if valid, rejecting default or -1 */
        Lora_AppendControllerGps(buffer, max_len, &app->gps);
    }

    size_t cur_len = strlen(buffer);
    if (cur_len < max_len - 2) {
        strncat(buffer, "\r\n", max_len - cur_len - 1);
    }
    return (uint16_t)strlen(buffer);
}

uint16_t Lora_StructureTxPacket(char *buffer, size_t max_len, uint8_t tier, const AppContext_t *app)
{
    if (app && app->is_controller && app->is_controller()) {
        return Lora_StructureControllerPacket(buffer, max_len, tier, app);
    }
    return Lora_StructurePlanePacket(buffer, max_len, tier, app);
}

uint8_t Lora_ExtractRemoteDataRate(const uint8_t *payload, uint16_t size, uint8_t default_dr)
{
    if (!payload || size == 0) {
        return default_dr;
    }
    char temp[256];
    uint16_t len = (size < sizeof(temp) - 1) ? size : sizeof(temp) - 1;
    memcpy(temp, payload, len);
    temp[len] = '\0';

    char *pdr = strstr(temp, "dr");
    if (pdr && *(pdr + 2) >= '0' && *(pdr + 2) <= '9') {
        return (uint8_t)strtol(pdr + 2, NULL, 10);
    }
    return default_dr;
}

uint16_t Lora_StructureRxReport(char *out_buf, size_t max_len, const uint8_t *payload, uint16_t size,
                                int16_t rssi, int8_t snr, uint8_t tier, const AppContext_t *app)
{
    if (!out_buf || max_len < 32) {
        return 0;
    }
    out_buf[0] = '\0';

    /* Copy and trim received payload */
    uint16_t copy_len = size;
    if (payload && size > 0) {
        while (copy_len > 0 && (payload[copy_len - 1] == '\r' ||
                                payload[copy_len - 1] == '\n' ||
                                payload[copy_len - 1] == ' '  ||
                                payload[copy_len - 1] == '\0')) {
            copy_len--;
        }
        if (copy_len > max_len - 128) {
            copy_len = max_len - 128;
        }
        memcpy(out_buf, payload, copy_len);
        out_buf[copy_len] = '\0';
    }

    /* Append reception RF metrics */
    char metrics_buf[128];
    snprintf(metrics_buf, sizeof(metrics_buf), ";rs%d;sn%d;dr%u;sf%u;bw%u;",
             rssi, snr, (unsigned int)tier,
             (unsigned int)ADR_GetSF(tier),
             (unsigned int)ADR_GetBW_kHz(tier));
    strncat(out_buf, metrics_buf, max_len - strlen(out_buf) - 1);

    /* Append controller GPS only if valid (rejecting default or -1) */
    if (app) {
        Lora_AppendControllerGps(out_buf, max_len, &app->gps);
    }

    /* Append line terminator */
    size_t cur_len = strlen(out_buf);
    if (cur_len < max_len - 2) {
        strncat(out_buf, "\r\n", max_len - cur_len - 1);
    }

    return (uint16_t)strlen(out_buf);
}
