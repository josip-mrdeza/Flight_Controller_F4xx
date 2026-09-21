#include "Drivers/i2c_helper.h"
#include "LCD/ssd1315.h"
#include <stdio.h>

HAL_StatusTypeDef I2C_WaitReady(I2C_HandleTypeDef *hi2c, uint32_t timeout) {
    uint32_t tickstart = HAL_GetTick();
    while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY) {
        if ((HAL_GetTick() - tickstart) > timeout) {
            hi2c->Instance->CR1 |= I2C_CR1_SWRST;
            hi2c->Instance->CR1 &= ~I2C_CR1_SWRST;
            hi2c->State = HAL_I2C_STATE_READY;
            return HAL_TIMEOUT;
        }
    }
    return HAL_OK;
}

HAL_StatusTypeDef I2C_Mem_Write_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    if (I2C_WaitReady(hi2c, Timeout) != HAL_OK) return HAL_BUSY;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write_DMA(hi2c, DevAddress, MemAddress, MemAddSize, pData, Size);
    if (status != HAL_OK) return status;
    return I2C_WaitReady(hi2c, Timeout);
}

HAL_StatusTypeDef I2C_Mem_Read_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    if (I2C_WaitReady(hi2c, Timeout) != HAL_OK) return HAL_BUSY;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read_DMA(hi2c, DevAddress, MemAddress, MemAddSize, pData, Size);
    if (status != HAL_OK) return status;
    return I2C_WaitReady(hi2c, Timeout);
}

HAL_StatusTypeDef I2C_Master_Transmit_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    if (I2C_WaitReady(hi2c, Timeout) != HAL_OK) return HAL_BUSY;
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit_DMA(hi2c, DevAddress, pData, Size);
    if (status != HAL_OK) return status;
    return I2C_WaitReady(hi2c, Timeout);
}

HAL_StatusTypeDef I2C_Master_Receive_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    if (I2C_WaitReady(hi2c, Timeout) != HAL_OK) return HAL_BUSY;
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive_DMA(hi2c, DevAddress, pData, Size);
    if (status != HAL_OK) return status;
    return I2C_WaitReady(hi2c, Timeout);
}

void Scan_I2C_Bus(I2C_HandleTypeDef *hi2c) {
	uint8_t found_devices = 0;
	SSD1315_Clear();
	for (uint16_t i = 1; i < 128; i++) {
		if (HAL_I2C_IsDeviceReady(hi2c, (i << 1), 2, 50) == HAL_OK) {
			found_devices++;
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
