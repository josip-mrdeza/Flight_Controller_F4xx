/**
  ******************************************************************************
  * @file           : lora_telemetry.h
  * @brief          : Telemetry data structuring, validation, and optimization
  *                   for LoRa packet transmission and reception.
  ******************************************************************************
  */

#ifndef __LORA_TELEMETRY_H
#define __LORA_TELEMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "main.h"

/* Default fallback GPS coordinates (used to detect uninitialized/default values) */
#define LORA_DEFAULT_GPS_LAT     45.815024f
#define LORA_DEFAULT_GPS_LON     15.981940f
#define LORA_DEFAULT_GPS_ALT     135.0f

/**
 * @brief Checks if a float value is valid (not NaN, not Inf, and not -1.0f).
 * @param val: Float value to check
 * @return true if valid, false if invalid or rejected
 */
bool Lora_IsValidFloat(float val);

/**
 * @brief Checks if GPS data has a valid fix, reasonable range, and is not default placeholder or -1.
 * @param gps: Pointer to GPS handle
 * @return true if GPS telemetry is valid, false otherwise
 */
bool Lora_IsValidGps(const GPS_HandleTypeDef *gps);

/**
 * @brief Subfunction to format and append GPS telemetry (lt, lo, al, sa).
 *        Rejects default coordinates, missing fixes, or invalid/-1 coordinates.
 * @param buffer: Output string buffer
 * @param max_len: Maximum capacity of output buffer
 * @param gps: Pointer to GPS handle
 * @return Number of characters appended
 */
uint16_t Lora_AppendGps(char *buffer, size_t max_len, const GPS_HandleTypeDef *gps);

/**
 * @brief Subfunction to format and append Controller GPS coordinates (clt, clo, cal).
 *        Rejects default coordinates, missing fixes, or invalid/-1 coordinates.
 * @param buffer: Output string buffer
 * @param max_len: Maximum capacity of output buffer
 * @param gps: Pointer to GPS handle
 * @return Number of characters appended
 */
uint16_t Lora_AppendControllerGps(char *buffer, size_t max_len, const GPS_HandleTypeDef *gps);

/**
 * @brief Subfunction to format and append kinematic velocity data (vx, vy, vz).
 *        Rejects values if invalid (NaN, Inf, or -1.0f).
 * @param buffer: Output string buffer
 * @param max_len: Maximum capacity of output buffer
 * @param vx: Velocity along X axis
 * @param vy: Velocity along Y axis
 * @param vz: Velocity along Z axis
 * @return Number of characters appended
 */
uint16_t Lora_AppendVelocities(char *buffer, size_t max_len, float vx, float vy, float vz);

/**
 * @brief Subfunction to format and append acceleration data (ax, ay, az).
 *        Rejects values if invalid (NaN, Inf, or -1.0f).
 * @param buffer: Output string buffer
 * @param max_len: Maximum capacity of output buffer
 * @param ax: Acceleration along X axis
 * @param ay: Acceleration along Y axis
 * @param az: Acceleration along Z axis
 * @return Number of characters appended
 */
uint16_t Lora_AppendAccelerations(char *buffer, size_t max_len, float ax, float ay, float az);

/**
 * @brief Subfunction to format and append attitude and heading (gx, gy, gz, hd).
 *        Rejects values if invalid (NaN, Inf, or -1.0f).
 * @param buffer: Output string buffer
 * @param max_len: Maximum capacity of output buffer
 * @param gx: Roll angle in degrees
 * @param gy: Pitch angle in degrees
 * @param gz: Yaw angle in degrees
 * @param hd: Heading in degrees
 * @return Number of characters appended
 */
uint16_t Lora_AppendAttitude(char *buffer, size_t max_len, float gx, float gy, float gz, float hd);

/**
 * @brief Subfunction to format and append environmental data (tm = temp, pr = pressure).
 *        Rejects values if invalid, unmeasured (0.0f temp), or -1.0f.
 * @param buffer: Output string buffer
 * @param max_len: Maximum capacity of output buffer
 * @param temp: Temperature in deg C
 * @param pressure: Barometric pressure
 * @return Number of characters appended
 */
uint16_t Lora_AppendEnvironment(char *buffer, size_t max_len, float temp, float pressure);

/**
 * @brief Subfunction to format and append power metrics (vb, ib, vs, is).
 *        Rejects values if unmeasured or -1.0f.
 * @param buffer: Output string buffer
 * @param max_len: Maximum capacity of output buffer
 * @param v_bat: Battery voltage
 * @param i_bat: Battery current
 * @param v_servo: Servo/solar voltage
 * @param i_servo: Servo/solar current
 * @return Number of characters appended
 */
uint16_t Lora_AppendPower(char *buffer, size_t max_len, float v_bat, float i_bat, float v_servo, float i_servo);

/**
 * @brief Structures the complete Plane telemetry packet for LoRa transmission.
 *        Optimizes payload length by rejecting any invalid, unmeasured, or default data.
 * @param buffer: Output string buffer
 * @param max_len: Maximum capacity of buffer
 * @param tier: Current target ADR data rate tier
 * @param app: Application context containing sensor handles
 * @return Total string length written to buffer
 */
uint16_t Lora_StructurePlanePacket(char *buffer, size_t max_len, uint8_t tier, const AppContext_t *app);

/**
 * @brief Structures the Controller packet for LoRa transmission.
 *        Optimizes payload length by rejecting default/invalid GPS coordinates.
 * @param buffer: Output string buffer
 * @param max_len: Maximum capacity of buffer
 * @param tier: Current target ADR data rate tier
 * @param app: Application context containing sensor handles
 * @return Total string length written to buffer
 */
uint16_t Lora_StructureControllerPacket(char *buffer, size_t max_len, uint8_t tier, const AppContext_t *app);

/**
 * @brief Automatically selects and structures the packet based on whether the device
 *        is operating as Controller or Plane.
 * @param buffer: Output string buffer
 * @param max_len: Maximum capacity of buffer
 * @param tier: Current target ADR data rate tier
 * @param app: Application context
 * @return Total string length written to buffer
 */
uint16_t Lora_StructureTxPacket(char *buffer, size_t max_len, uint8_t tier, const AppContext_t *app);

/**
 * @brief Extracts the remote data rate ('dr' tag) from a received LoRa payload.
 * @param payload: Received payload buffer
 * @param size: Payload length in bytes
 * @param default_dr: Fallback data rate if not found
 * @return Parsed data rate tier
 */
uint8_t Lora_ExtractRemoteDataRate(const uint8_t *payload, uint16_t size, uint8_t default_dr);

/**
 * @brief Formats the received LoRa payload and appends reception RF metrics (rs, sn, dr, sf, bw)
 *        and controller GPS (if valid) for USB/CDC OTG output.
 * @param out_buf: Output string buffer
 * @param max_len: Maximum capacity of buffer
 * @param payload: Received payload
 * @param size: Payload length in bytes
 * @param rssi: Packet RSSI
 * @param snr: Packet SNR
 * @param tier: Current ADR tier
 * @param app: Application context
 * @return Total string length written to out_buf
 */
uint16_t Lora_StructureRxReport(char *out_buf, size_t max_len, const uint8_t *payload, uint16_t size,
                                int16_t rssi, int8_t snr, uint8_t tier, const AppContext_t *app);

#ifdef __cplusplus
}
#endif

#endif /* __LORA_TELEMETRY_H */
