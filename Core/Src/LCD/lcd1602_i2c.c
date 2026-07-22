#include "LCD/lcd1602_i2c.h"

#include "stm32f4xx_hal.h"
static I2C_HandleTypeDef *lcd_i2c;

#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE    0x04
static uint8_t lcd_addr = 0;



#define LCD_CMD  0
#define LCD_DATA 1

void lcd_print(char* ptr, uint8_t line)
{
	lcd_set_cursor(0, line);

	while (*ptr)
		lcd_write_byte(((uint8_t)*ptr++), 1);
}
void lcd_clear()
{
	lcd_cmd(0x01);
	HAL_Delay(2);
}


static void lcd_write_nibble(uint8_t nibble, uint8_t mode)
{
	uint8_t data = nibble | LCD_BACKLIGHT | (mode ? 0x01 : 0x00);

	HAL_I2C_Master_Transmit(lcd_i2c, lcd_addr, &data, 1, 10);

	data |= LCD_ENABLE;
	HAL_I2C_Master_Transmit(lcd_i2c, lcd_addr, &data, 1, 10);

	data &= ~LCD_ENABLE;
	HAL_I2C_Master_Transmit(lcd_i2c, lcd_addr, &data, 1, 10);

	HAL_Delay(1);
}

void lcd_write_byte(uint8_t byte, uint8_t mode)
{
	lcd_write_nibble(byte & 0xF0, mode);
	lcd_write_nibble((byte << 4) & 0xF0, mode);
}

void lcd_cmd(uint8_t cmd)
{
	lcd_write_byte(cmd, LCD_CMD);
}

void disable_lcd(void)
{
	lcd_cmd(0x08);


	uint8_t data = 0x00;   // no backlight, no RS, no EN
	HAL_I2C_Master_Transmit(lcd_i2c, lcd_addr, &data, 1, 10);
}

void init_lcd(I2C_HandleTypeDef *hi2c)
{
	lcd_i2c = hi2c;

	lcd_addr = 0x27 << 1;
	HAL_Delay(50);

	lcd_write_nibble(0x30, LCD_CMD);
	HAL_Delay(5);
	lcd_write_nibble(0x30, LCD_CMD);
	HAL_Delay(1);
	lcd_write_nibble(0x30, LCD_CMD);
	HAL_Delay(10);

	lcd_write_nibble(0x20, LCD_CMD); // 4-bit mode

	lcd_cmd(0x28); // 4-bit, 2-line
	lcd_cmd(0x0C); // Display ON, cursor OFF
	lcd_cmd(0x06); // Auto-increment
	lcd_cmd(0x01); // Clear
	HAL_Delay(2);
}

void lcd_set_cursor(uint8_t col, uint8_t row)
{
	uint8_t row_offsets[] = {0x00, 0x40};
	lcd_cmd(0x80 | (col + row_offsets[row]));
}
