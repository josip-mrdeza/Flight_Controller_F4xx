#ifndef CC1101_H
#define CC1101_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* ========================================================================== */
/*                            MACROS & CONSTANTS                              */
/* ========================================================================== */

#define CC1101_FIFO_SIZE             64

/* SPI Access Types */
#define CC1101_WRITE_BURST           0x40
#define CC1101_READ_SINGLE           0x80
#define CC1101_READ_BURST            0xC0

/* Configuration Registers */
#define CC1101_REG_IOCFG2            0x00
#define CC1101_REG_IOCFG1            0x01
#define CC1101_REG_IOCFG0            0x02
#define CC1101_REG_FIFOTHR           0x03
#define CC1101_REG_SYNC1             0x04
#define CC1101_REG_SYNC0             0x05
#define CC1101_REG_PKTLEN            0x06
#define CC1101_REG_PKTCTRL1          0x07
#define CC1101_REG_PKTCTRL0          0x08
#define CC1101_REG_ADDR              0x09
#define CC1101_REG_CHANNR            0x0A
#define CC1101_REG_FSCTRL1           0x0B
#define CC1101_REG_FSCTRL0           0x0C
#define CC1101_REG_FREQ2             0x0D
#define CC1101_REG_FREQ1             0x0E
#define CC1101_REG_FREQ0             0x0F
#define CC1101_REG_MDMCFG4           0x10
#define CC1101_REG_MDMCFG3           0x11
#define CC1101_REG_MDMCFG2           0x12
#define CC1101_REG_MDMCFG1           0x13
#define CC1101_REG_MDMCFG0           0x14
#define CC1101_REG_DEVIATN           0x15
#define CC1101_REG_MCSM2             0x16
#define CC1101_REG_MCSM1             0x17
#define CC1101_REG_MCSM0             0x18
#define CC1101_REG_FOCCFG            0x19
#define CC1101_REG_BSCFG             0x1A
#define CC1101_REG_AGCCTRL2          0x1B
#define CC1101_REG_AGCCTRL1          0x1C
#define CC1101_REG_AGCCTRL0          0x1D
#define CC1101_REG_WOREVT1           0x1E
#define CC1101_REG_WOREVT0           0x1F
#define CC1101_REG_WORCTRL           0x20
#define CC1101_REG_FREND1            0x21
#define CC1101_REG_FREND0            0x22
#define CC1101_REG_FSCAL3            0x23
#define CC1101_REG_FSCAL2            0x24
#define CC1101_REG_FSCAL1            0x25
#define CC1101_REG_FSCAL0            0x26
#define CC1101_REG_RCCTRL1           0x27
#define CC1101_REG_RCCTRL0           0x28
#define CC1101_REG_FSTEST            0x29
#define CC1101_REG_PTEST             0x2A
#define CC1101_REG_AGCTEST           0x2B
#define CC1101_REG_TEST2             0x2C
#define CC1101_REG_TEST1             0x2D
#define CC1101_REG_TEST0             0x2E

/* Command Strobes */
#define CC1101_STROBE_SRES           0x30  // Reset
#define CC1101_STROBE_SFSTXON        0x31  // Enable/calibrate frequency synthesizer
#define CC1101_STROBE_SXOFF          0x32  // Turn off crystal oscillator
#define CC1101_STROBE_SCAL           0x33  // Calibrate frequency synthesizer
#define CC1101_STROBE_SRX            0x34  // Enable RX
#define CC1101_STROBE_STX            0x35  // Enable TX
#define CC1101_STROBE_SIDLE          0x36  // Enter IDLE state
#define CC1101_STROBE_SWOR           0x38  // Start Wake-on-Radio
#define CC1101_STROBE_SPWD           0x39  // Enter power down mode
#define CC1101_STROBE_SFRX           0x3A  // Flush the RX FIFO
#define CC1101_STROBE_SFTX           0x3B  // Flush the TX FIFO
#define CC1101_STROBE_SWORRST        0x3C  // Reset RTC to Event1
#define CC1101_STROBE_SNOP           0x3D  // No operation (used to read status)

/* Status Registers */
#define CC1101_STATUS_PARTNUM        0x30
#define CC1101_STATUS_VERSION        0x31
#define CC1101_STATUS_FREQEST        0x32
#define CC1101_STATUS_LQI            0x33
#define CC1101_STATUS_RSSI           0x34
#define CC1101_STATUS_MARCSTATE      0x35
#define CC1101_STATUS_WORTIME1       0x36
#define CC1101_STATUS_WORTIME0       0x37
#define CC1101_STATUS_PKTSTATUS      0x38
#define CC1101_STATUS_VCO_VC_DAC     0x39
#define CC1101_STATUS_TXBYTES        0x3A
#define CC1101_STATUS_RXBYTES        0x3B

/* FIFO Access Registers */
#define CC1101_REG_TXFIFO            0x3F
#define CC1101_REG_RXFIFO            0x3F

/* ========================================================================== */
/*                             DATA STRUCTURES                                */
/* ========================================================================== */

/**
 * @brief CC1101 internal state machine representation
 */
typedef enum {
    CC1101_STATE_IDLE = 0,
    CC1101_STATE_RX,
    CC1101_STATE_TX,
    CC1101_STATE_SLEEP,
    CC1101_STATE_CALIBRATING,
    CC1101_STATE_ERROR
} cc1101_state_t;

/**
 * @brief CC1101 Device Handle Structure
 */
typedef struct {
    /* Hardware Interface */
    SPI_HandleTypeDef *hspi;       // Pointer to SPI3 handle

    GPIO_TypeDef *cs_port;         // Chip Select Port
    uint16_t cs_pin;               // Chip Select Pin

    GPIO_TypeDef *gdo0_port;       // GDO0 Port (Interrupt)
    uint16_t gdo0_pin;             // GDO0 Pin

    GPIO_TypeDef *gdo2_port;       // GDO2 Port (Interrupt)
    uint16_t gdo2_pin;             // GDO2 Pin

    /* Data Buffers */
    uint8_t tx_fifo[CC1101_FIFO_SIZE];
    uint8_t rx_fifo[CC1101_FIFO_SIZE];

    /* Variables */
    uint8_t rx_len;                // Length of recently received packet
    int8_t rssi;                   // Received Signal Strength Indicator
    uint8_t lqi;                   // Link Quality Indicator

    cc1101_state_t state;          // Current operational state

    /* DMA Management */
    volatile bool dma_busy;        // Used to track when DMA is operating
} cc1101_t;

/* ========================================================================== */
/*                             FUNCTION PROTOTYPES                            */
/* ========================================================================== */

/**
 * @brief  Initializes the CC1101 module with 433MHz defaults
 * @param  dev: Pointer to the cc1101_t device handle
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef CC1101_Init(cc1101_t *dev);

/**
 * @brief  Resets the CC1101 via SPI strobe
 */
void CC1101_Reset(cc1101_t *dev);

/**
 * @brief  Sends a strobe command
 */
uint8_t CC1101_Strobe(cc1101_t *dev, uint8_t strobe);

/**
 * @brief  Puts the module into Receive mode
 */
void CC1101_SetRX(cc1101_t *dev);

/**
 * @brief  Puts the module into Transmit mode
 */
void CC1101_SetTX(cc1101_t *dev);

/**
 * @brief  Transmits data using SPI3 DMA
 * @param  dev: Pointer to device handle
 * @param  data: Data array to transmit
 * @param  len: Length of data
 */
HAL_StatusTypeDef CC1101_Transmit_DMA(cc1101_t *dev, uint8_t *data, uint8_t len);

/**
 * @brief  Reads received data using SPI3 DMA
 * @param  dev: Pointer to device handle
 */
HAL_StatusTypeDef CC1101_Receive_DMA(cc1101_t *dev);

/**
 * @brief  Call this inside the HAL_SPI_TxRxCpltCallback / HAL_SPI_TxCpltCallback
 *         to raise the CS pin when the DMA transaction finishes.
 */
void CC1101_DMA_Complete_Callback(cc1101_t *dev);

/**
 * @brief  Call this in your EXTI IRQ handler linked to the GDO0/GDO2 pin
 */
void CC1101_Interrupt_Handler(cc1101_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* CC1101_H */
