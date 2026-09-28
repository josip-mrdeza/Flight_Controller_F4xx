#ifndef __USER_CONFIG_H
#define __USER_CONFIG_H

#include "stm32f4xx_hal.h"
#include "sx126x.h"
#include "Drivers/DX-LR30_Driver/adr.h"

#define TEST 0

#define LCC68_NSS_PORT   GPIOD
#define LCC68_NSS_PIN    GPIO_PIN_2

#define LCC68_SCK_PORT   GPIOC
#define LCC68_SCK_PIN    GPIO_PIN_10

#define LCC68_MOSI_PORT  GPIOC
#define LCC68_MOSI_PIN   GPIO_PIN_12

#define LCC68_MISO_PORT  GPIOC
#define LCC68_MISO_PIN   GPIO_PIN_11

#define LCC68_NRST_PORT  GPIOB
#define LCC68_NRST_PIN   GPIO_PIN_5

#define LCC68_BUSY_PORT  GPIOB
#define LCC68_BUSY_PIN   GPIO_PIN_6

#define LCC68_DIO1_PORT  GPIOB
#define LCC68_DIO1_PIN   GPIO_PIN_3

#define LCC68_DIO2_PORT  GPIOB
#define LCC68_DIO2_PIN   GPIO_PIN_4

#define LCC68_RXEN_PORT  GPIOB
#define LCC68_RXEN_PIN   GPIO_PIN_7

#define LCC68_TXEN_PORT  GPIOB
#define LCC68_TXEN_PIN   GPIO_PIN_8

#define LORA_FRE         869400000
#define LORA_PREAMBLE_LENGTH 8
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define SIZE_DATA        255

typedef enum
{
    MODE_SLEEP = 0x00,
    MODE_STDBY_RC,
    MODE_STDBY_XOSC,
    MODE_FS,
    MODE_TX,
    MODE_RX,
    MODE_RX_DC,
    MODE_CAD
} RadioOperatingModes_t;

extern uint8_t rxbuff[SIZE_DATA];
extern uint8_t DataLen;
extern uint8_t pdata;

extern volatile uint8_t IrqFired;
extern sx126x_irq_mask_t radioFlag;
extern sx126x_rx_buffer_status_t offset;
extern sx126x_pkt_status_lora_t RadioPktStatus;
extern volatile uint32_t lastTransmitDelay;
extern volatile uint32_t tickTransmitStart;
extern volatile uint32_t tickTransmitEnd;
extern volatile uint8_t lastTransmitLength;
extern volatile float approxDataTransferSpeed;

extern volatile uint32_t LORA_SX126x_SYMBOL_TIMEOUT;
uint32_t SX126x_CalcSymbolTimeout(void);
uint32_t SX126x_BW_Hz(uint8_t bw);
uint32_t SX126x_TimeoutMs_To_Symbols(uint32_t timeout_ms);

void Data_Processing(void);
void LoraInit(void);
HAL_StatusTypeDef DX_LR30_Init(void);
_Bool DX_LR30_Ping(void);
void set_LoraPacketParams(uint8_t size);

void Radio_process(void);
void LoraDataSend(uint8_t *data, uint8_t len);
void LoraOpenRXMode(uint32_t Timerout);
void DX_Lora_RadioIrqProcess(void);
void UpdateMenuADR(uint8_t state, int16_t rssi, int8_t snr);

RadioOperatingModes_t sx1262GetOperatingMode(void);
void sx1262SetOperatingMode(RadioOperatingModes_t mode);

#endif
