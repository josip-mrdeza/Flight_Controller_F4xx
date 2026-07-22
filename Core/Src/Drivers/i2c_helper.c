#include "Drivers/i2c_helper.h"
void Scan_I2C_Bus(I2C_HandleTypeDef *hi2c) {
	uint8_t found_devices = 0;
	SSD1315_Clear();
	for (uint16_t i = 1; i < 128; i++) {
		// HAL expects left-shifted 8-bit address
		if (HAL_I2C_IsDeviceReady(hi2c, (i << 1), 2, 50) == HAL_OK) {
			found_devices++;
			// Put a breakpoint on the line below!
			// 'i' will be your 7-bit address in Hex (e.g., 0x40 or 0x70)
			uint8_t detected_addr = i;
			(void)detected_addr;
			char buff[24];
			sprintf(buff, "Addr: %d", detected_addr);
			SSD1315_Line_1(buff);
			SSD1315_UpdateScreen(&hi2c3);
			HAL_Delay(250);
		}
	}
}
