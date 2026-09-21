/**
  ******************************************************************************
  * @file    gy_gps6mv2.c
  * @brief   Driver source implementation for GY-GPS6MV2 (u-blox NEO-6M)
  ******************************************************************************
  */

#include "Drivers/gy_gps6mv2.h"
#include "LCD/ssd1315.h"
#include "LCD/menu_helper.h"
#include "i2c.h"
#include "main.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void GPS_ParseSentence(GPS_HandleTypeDef *hgps, char *line);
static void GPS_ParseGPRMC(GPS_HandleTypeDef *hgps, char *line);
static void GPS_ParseGPGGA(GPS_HandleTypeDef *hgps, char *line);
static float GPS_NMEAToDecimal(const char *raw, char dir);

HAL_StatusTypeDef GPS_Init(GPS_HandleTypeDef *hgps, UART_HandleTypeDef *huart) {
    if (!hgps || !huart) return HAL_ERROR;

    __HAL_UART_DISABLE_IT(huart, UART_IT_IDLE);
    HAL_UART_DMAStop(huart);

    memset(hgps, 0, sizeof(GPS_HandleTypeDef));
    hgps->huart = huart;

    volatile uint32_t tmpreg = huart->Instance->SR;
    tmpreg = huart->Instance->DR;
    (void)tmpreg;
    __HAL_UART_CLEAR_OREFLAG(huart);
    __HAL_UART_CLEAR_IDLEFLAG(huart);

    HAL_StatusTypeDef status = HAL_UART_Receive_DMA(hgps->huart, hgps->dma_rx_buf, GPS_DMA_BUF_SIZE);
    if (status == HAL_OK) {
        __HAL_UART_CLEAR_IDLEFLAG(hgps->huart);
        __HAL_UART_ENABLE_IT(hgps->huart, UART_IT_IDLE);
    }

    char buff[32];
    snprintf(buff, sizeof(buff), "[GPS-NEO6M - INIT]");
    SSD1315_Title(buff);
    snprintf(buff, sizeof(buff), "Init Gps: %s", (status == HAL_OK) ? "Ok" : "Fail");
    if(status != HAL_OK){
    	HAL_Delay(500);
    }
    SSD1315_Line_2(buff);
    SSD1315_UpdateScreen(&hi2c3);

    return status;
}

void GPS_ProcessBuffer(GPS_HandleTypeDef *hgps) {
    if (!hgps || !hgps->huart || !hgps->huart->hdmarx) return;

    uint16_t wr_ptr = (GPS_DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(hgps->huart->hdmarx)) % GPS_DMA_BUF_SIZE;

    while (hgps->rd_ptr != wr_ptr) {
        char c = (char)hgps->dma_rx_buf[hgps->rd_ptr];
        hgps->rd_ptr = (hgps->rd_ptr + 1) % GPS_DMA_BUF_SIZE;

        if (c == '$') {
            hgps->line_idx = 0;
        }

        if (hgps->line_idx < GPS_NMEA_LINE_MAX - 1) {
            hgps->line_buf[hgps->line_idx++] = c;
        }

        if (c == '\n') {
            hgps->line_buf[hgps->line_idx] = '\0';
            GPS_ParseSentence(hgps, hgps->line_buf);
            hgps->line_idx = 0;
        }
    }
}

GPS_Data_t GPS_GetData(GPS_HandleTypeDef *hgps) {
    return hgps->data;
}

void GPS_SetUpdateRate5Hz(GPS_HandleTypeDef *hgps) {
    /* UBX-CFG-RATE payload to set measurement rate to 200ms (5Hz) */
    static const uint8_t ubx_5hz[] = {
        0xB5, 0x62,             /* Sync Chars */
        0x06, 0x08,             /* Class: CFG, ID: RATE */
        0x06, 0x00,             /* Payload length: 6 bytes */
        0xC8, 0x00,             /* Meas Rate: 200 ms */
        0x01, 0x00,             /* Nav Rate: 1 cycle */
        0x01, 0x00,             /* Time Ref: GPS UTC */
        0xDE, 0x6A              /* Checksum CK_A, CK_B */
    };
    HAL_UART_Transmit(hgps->huart, (uint8_t*)ubx_5hz, sizeof(ubx_5hz), 100);
}

/* --- Internal Parsing Helper Functions --- */

static float GPS_NMEAToDecimal(const char *raw, char dir) {
    if (!raw || strlen(raw) == 0) return 0.0f;

    float raw_val = strtof(raw, NULL);
    int degrees = (int)(raw_val / 100.0f);
    float minutes = raw_val - (degrees * 100.0f);
    float decimal = (float)degrees + (minutes / 60.0f);

    if (dir == 'S' || dir == 'W') {
        decimal = -decimal;
    }
    return decimal;
}

static char* next_field(char **cursor) {
    if (!cursor || !*cursor) return NULL;
    char *start = *cursor;
    char *comma = strchr(start, ',');
    if (comma) {
        *comma = '\0';
        *cursor = comma + 1;
    } else {
        char *star = strchr(start, '*');
        if (star) *star = '\0';
        char *cr = strchr(start, '\r');
        if (cr) *cr = '\0';
        char *lf = strchr(start, '\n');
        if (lf) *lf = '\0';
        *cursor = NULL;
    }
    return start;
}

static void GPS_ParseSentence(GPS_HandleTypeDef *hgps, char *line) {
    if (strncmp(line, "$GPRMC", 6) == 0 || strncmp(line, "$GNRMC", 6) == 0) {
        GPS_ParseGPRMC(hgps, line);
    } else if (strncmp(line, "$GPGGA", 6) == 0 || strncmp(line, "$GNGGA", 6) == 0) {
        GPS_ParseGPGGA(hgps, line);
    }
}

static void GPS_ParseGPRMC(GPS_HandleTypeDef *hgps, char *line) {
    char line_copy[GPS_NMEA_LINE_MAX];
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';

    char *cursor = line_copy;
    char *token;
    int field = 0;

    char status = 'V';
    char lat_str[16] = {0}, lat_dir = 'N';
    char lon_str[16] = {0}, lon_dir = 'E';
    char speed_str[12] = {0};
    char time_str[12] = {0};

    while ((token = next_field(&cursor)) != NULL) {
        switch (field) {
            case 1: strncpy(time_str, token, sizeof(time_str) - 1); break;
            case 2: status = (token[0] != '\0') ? token[0] : 'V'; break;
            case 3: strncpy(lat_str, token, sizeof(lat_str) - 1); break;
            case 4: lat_dir = (token[0] != '\0') ? token[0] : 'N'; break;
            case 5: strncpy(lon_str, token, sizeof(lon_str) - 1); break;
            case 6: lon_dir = (token[0] != '\0') ? token[0] : 'E'; break;
            case 7: strncpy(speed_str, token, sizeof(speed_str) - 1); break;
            default: break;
        }
        field++;
    }

    if (status == 'A') {
        hgps->data.is_valid = true;
        if (strlen(lat_str) > 0) hgps->data.latitude = GPS_NMEAToDecimal(lat_str, lat_dir);
        if (strlen(lon_str) > 0) hgps->data.longitude = GPS_NMEAToDecimal(lon_str, lon_dir);
        if (strlen(speed_str) > 0) {
            hgps->data.speed_knots = strtof(speed_str, NULL);
            hgps->data.speed_kmh = hgps->data.speed_knots * 1.852f;
        }
        hgps->last_update_ms = HAL_GetTick();

        if (strlen(time_str) >= 6) {
            hgps->data.time.hours   = (time_str[0] - '0') * 10 + (time_str[1] - '0');
            hgps->data.time.minutes = (time_str[2] - '0') * 10 + (time_str[3] - '0');
            hgps->data.time.seconds = (time_str[4] - '0') * 10 + (time_str[5] - '0');
        }
    } else {
        hgps->data.is_valid = false;
    }
}

static void GPS_ParseGPGGA(GPS_HandleTypeDef *hgps, char *line) {
    char line_copy[GPS_NMEA_LINE_MAX];
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';

    char *cursor = line_copy;
    char *token;
    int field = 0;

    char lat_str[16] = {0}, lat_dir = 'N';
    char lon_str[16] = {0}, lon_dir = 'E';
    char fix_quality = '0';
    char alt_str[12] = {0};
    char sats_str[8] = {0};

    while ((token = next_field(&cursor)) != NULL) {
        switch (field) {
            case 2: strncpy(lat_str, token, sizeof(lat_str) - 1); break;
            case 3: lat_dir = (token[0] != '\0') ? token[0] : 'N'; break;
            case 4: strncpy(lon_str, token, sizeof(lon_str) - 1); break;
            case 5: lon_dir = (token[0] != '\0') ? token[0] : 'E'; break;
            case 6: fix_quality = (token[0] != '\0') ? token[0] : '0'; break;
            case 7: strncpy(sats_str, token, sizeof(sats_str) - 1); break;
            case 9: strncpy(alt_str, token, sizeof(alt_str) - 1); break;
            default: break;
        }
        field++;
    }

    if (strlen(sats_str) > 0) {
        hgps->data.satellites = (uint8_t)atoi(sats_str);
    }

    if (fix_quality != '0') {
        hgps->data.is_valid = true;
        if (strlen(lat_str) > 0) hgps->data.latitude = GPS_NMEAToDecimal(lat_str, lat_dir);
        if (strlen(lon_str) > 0) hgps->data.longitude = GPS_NMEAToDecimal(lon_str, lon_dir);
        if (strlen(alt_str) > 0) hgps->data.altitude = strtof(alt_str, NULL);
        hgps->last_update_ms = HAL_GetTick();
    }
}

void gps_update_dma(void)
{
	if (g_app.is_controller && g_app.is_controller()) {
		return;
	}

	GPS_ProcessBuffer(&g_app.gps);

	char buff[32];
	if (g_app.gps.data.is_valid) {
		if(true)
		{
			return;
		}
		float lat = g_app.gps.data.latitude;
		float lon = g_app.gps.data.longitude;
		float alt = g_app.gps.data.altitude;

		snprintf(buff, sizeof(buff), "[GPS-FIX %d SATS]", g_app.gps.data.satellites);
		SSD1315_Title(buff);
		snprintf(buff, sizeof(buff), "lat:%.5f", lat);
		SSD1315_Line_1(buff);
		snprintf(buff, sizeof(buff), "lon:%.5f", lon);
		SSD1315_Line_2(buff);
		snprintf(buff, sizeof(buff), "alt:%.2f", alt);
		SSD1315_Line_3(buff);
		SSD1315_UpdateScreen(&hi2c3);
		return;
	}

	if(display_off)
	{
		return;
	}
//	snprintf(buff, sizeof(buff), "[GPS:NO LOCK]");
//	SSD1315_Title(buff);
//	snprintf(buff, sizeof(buff), "Sats: %d", g_app.gps.data.satellites);
//	SSD1315_Line_1(buff);
//	snprintf(buff, sizeof(buff), "RxBuf: %u", g_app.gps.rd_ptr);
//	SSD1315_Line_2(buff);
//	SSD1315_Line_3("Searching GPS...");
//	SSD1315_UpdateScreen(&hi2c3);
}
