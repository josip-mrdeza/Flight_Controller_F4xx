#ifndef LCD1602_I2C_H
#define LCD1602_I2C_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/*
 */

// Initialize LCD with given I2C handle
void init_lcd(I2C_HandleTypeDef *hi2c);

// Clear entire display
void lcd_clear(void);

// Print a null‑terminated string on a specific line (0 or 1)
void lcd_print(char *ptr, uint8_t line);

// Set cursor to column/row
void lcd_set_cursor(uint8_t col, uint8_t row);

// Disable LCD (display OFF + backlight OFF)
void disable_lcd(void);

#endif // LCD1602_I2C_H
