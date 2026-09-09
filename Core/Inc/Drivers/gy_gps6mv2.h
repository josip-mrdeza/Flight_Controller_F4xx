/**
  ******************************************************************************
  * @file    gy_gps6mv2.h
  * @brief   Driver header for GY-GPS6MV2 (u-blox NEO-6M) on STM32F405
  *          Optimized for UART DMA Circular Buffer parsing.
  ******************************************************************************
  */

#ifndef GY_GPS6MV2_H
#define GY_GPS6MV2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define GPS_DMA_BUF_SIZE    256   /* Size of DMA circular ring buffer */
#define GPS_NMEA_LINE_MAX   128   /* Maximum length of single NMEA sentence */

typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} GPS_Time_t;

typedef struct {
    float latitude;       /* Decimal degrees (+ North, - South) */
    float longitude;      /* Decimal degrees (+ East, - West) */
    float altitude;       /* Altitude above sea level in meters */
    float speed_knots;    /* Speed in knots */
    float speed_kmh;      /* Speed in km/h */
    uint8_t satellites;   /* Number of satellites in view/used */
    bool is_valid;        /* Fix validity flag (true = 2D/3D fix) */
    GPS_Time_t time;      /* UTC Time */
} GPS_Data_t;

typedef struct {
    UART_HandleTypeDef *huart;              /* Pointer to HAL UART handle */
    uint8_t dma_rx_buf[GPS_DMA_BUF_SIZE];   /* Circular DMA reception buffer */
    uint16_t rd_ptr;                        /* Local read position pointer */
    char line_buf[GPS_NMEA_LINE_MAX];       /* Sentence accumulation buffer */
    uint8_t line_idx;                       /* Index for sentence buffer */
    GPS_Data_t data;                        /* Latest parsed GPS telemetry */
    uint32_t last_update_ms;                /* HAL Tick of last valid fix */
} GPS_HandleTypeDef;

/**
 * @brief  Initialize GPS handle, enable IDLE interrupt, and start DMA.
 * @param  hgps: Pointer to GPS_HandleTypeDef
 * @param  huart: Pointer to UART_HandleTypeDef (e.g., &huart1)
 * @return HAL status
 */
HAL_StatusTypeDef GPS_Init(GPS_HandleTypeDef *hgps, UART_HandleTypeDef *huart);

/**
 * @brief  Processes newly arrived bytes in the circular DMA buffer.
 *         Call this in main loop or inside UART IDLE IRQ / Callback.
 * @param  hgps: Pointer to GPS_HandleTypeDef
 */
void GPS_ProcessBuffer(GPS_HandleTypeDef *hgps);

/**
 * @brief  Get current decoded GPS data.
 * @param  hgps: Pointer to GPS_HandleTypeDef
 * @return Copy of latest GPS_Data_t structure
 */
GPS_Data_t GPS_GetData(GPS_HandleTypeDef *hgps);

/**
 * @brief  Optional helper to request 5Hz update rate from u-blox NEO-6M module.
 * @param  hgps: Pointer to GPS_HandleTypeDef
 */
void GPS_SetUpdateRate5Hz(GPS_HandleTypeDef *hgps);
void gps_update_dma(void);

#ifdef __cplusplus
}
#endif

#endif /* GY_GPS6MV2_H */
