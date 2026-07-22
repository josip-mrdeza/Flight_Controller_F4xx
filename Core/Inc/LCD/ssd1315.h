/*
 * ssd1315.h
 *
 *  Created on: Jun 24, 2026
 *      Author: joki
 */


#ifndef __SSD1315_H__
#define __SSD1315_H__

#include "stm32f4xx_hal.h"

#define SSD1315_I2C_ADDR        (0x3C << 1) //7bit i2c addy

#define SSD1315_WIDTH           128
#define SSD1315_HEIGHT          64

uint8_t SSD1315_Init(I2C_HandleTypeDef *hi2c);
void SSD1315_UpdateScreen(I2C_HandleTypeDef *hi2c);
void SSD1315_Clear(void);
void SSD1315_DrawPixel(uint8_t x, uint8_t y, uint8_t color);

typedef enum {
	STATE_INIT,
	STATE_RX_RADIO,
	STATE_TX_RADIO,
	STATE_GYROSCOPE
} MenuState_t;

typedef struct {
    MenuState_t currentState;
    uint8_t selectedItem;
    uint8_t* ptrToUsbBuff;
    uint16_t* ptrToUsbBuffLen;
} AppData_t;

void SSD1315_DrawChar(uint8_t x, uint8_t y, char c);
void SSD1315_DrawString(uint8_t x, uint8_t y, const char* str);
void SSD1315_Title(const char* str);
void SSD1315_Line_1(const char* str);
void SSD1315_Line_2(const char* str);
void SSD1315_Line_3(const char* str);
void Menu_HandleButtonClick(AppData_t *data);
void Menu_HandleButtonEncoder(AppData_t *data);

#endif /* __SSD1315_H__ */
