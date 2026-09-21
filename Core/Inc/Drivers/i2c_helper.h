#ifndef INC_DRIVERS_I2C_HELPER_H_
#define INC_DRIVERS_I2C_HELPER_H_

#ifndef __I2C_H__
#include "i2c.h"
#endif

HAL_StatusTypeDef I2C_WaitReady(I2C_HandleTypeDef *hi2c, uint32_t timeout);
HAL_StatusTypeDef I2C_Mem_Write_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef I2C_Mem_Read_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef I2C_Master_Transmit_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef I2C_Master_Receive_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
void Scan_I2C_Bus(I2C_HandleTypeDef *hi2c);

#endif
