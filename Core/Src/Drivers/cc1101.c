#include "Drivers/cc1101.h"

/* ========================================================================== */
/*                             INTERNAL FUNCTIONS                             */
/* ========================================================================== */

static void CS_Low(cc1101_t *dev) {
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);
}

static void CS_High(cc1101_t *dev) {
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
}

static uint8_t CC1101_WriteReg(cc1101_t *dev, uint8_t reg, uint8_t val) {
    uint8_t tx_buf[2] = {reg, val};
    uint8_t rx_buf[2] = {0};

    CS_Low(dev);
    HAL_SPI_TransmitReceive(dev->hspi, tx_buf, rx_buf, 2, HAL_MAX_DELAY);
    CS_High(dev);

    return rx_buf[0]; // Return status byte
}

static uint8_t CC1101_ReadReg(cc1101_t *dev, uint8_t reg) {
    uint8_t tx_buf[2] = {reg | CC1101_READ_SINGLE, 0x00};
    uint8_t rx_buf[2] = {0};

    CS_Low(dev);
    HAL_SPI_TransmitReceive(dev->hspi, tx_buf, rx_buf, 2, HAL_MAX_DELAY);
    CS_High(dev);

    return rx_buf[1];
}

static uint8_t CC1101_ReadStatus(cc1101_t *dev, uint8_t reg) {
    uint8_t tx_buf[2] = {reg | CC1101_READ_BURST, 0x00};
    uint8_t rx_buf[2] = {0};

    CS_Low(dev);
    HAL_SPI_TransmitReceive(dev->hspi, tx_buf, rx_buf, 2, HAL_MAX_DELAY);
    CS_High(dev);

    return rx_buf[1];
}

/* ========================================================================== */
/*                                PUBLIC API                                  */
/* ========================================================================== */

uint8_t CC1101_Strobe(cc1101_t *dev, uint8_t strobe) {
    uint8_t status = 0;
    CS_Low(dev);
    HAL_SPI_TransmitReceive(dev->hspi, &strobe, &status, 1, HAL_MAX_DELAY);
    CS_High(dev);
    return status;
}

void CC1101_Reset(cc1101_t *dev) {
    CS_High(dev);
    HAL_Delay(1);
    CS_Low(dev);
    HAL_Delay(1);
    CS_High(dev);
    HAL_Delay(1);

    CC1101_Strobe(dev, CC1101_STROBE_SRES);
    HAL_Delay(5);
    dev->state = CC1101_STATE_IDLE;
}

HAL_StatusTypeDef CC1101_Init(cc1101_t *dev) {
    CC1101_Reset(dev);

    /* ---------------------------------------------------------------------- */
    /* 1. HARDWARE VERIFICATION                                               */
    /* ---------------------------------------------------------------------- */

    // Check Part Number and Version
    // PARTNUM is always 0x00 on CC1101. VERSION is typically 0x14 (or 0x04).
    uint8_t version = CC1101_ReadStatus(dev, CC1101_STATUS_VERSION);

    // If MISO is floating or shorted, it will likely return 0xFF or 0x00
    if (version == 0x00 || version == 0xFF) {
        return HAL_ERROR;
    }

    /* ---------------------------------------------------------------------- */
    /* 2. REGISTER CONFIGURATION                                              */
    /* ---------------------------------------------------------------------- */

    // GDO0 configured to assert on sync word, de-assert at end of packet.
    CC1101_WriteReg(dev, CC1101_REG_IOCFG0,   0x06);
    // GDO2 as CHIP_RDYn
    CC1101_WriteReg(dev, CC1101_REG_IOCFG2,   0x29);

    /* ---------------------------------------------------------------------- */
    /* 3. WRITE/READ VERIFICATION                                             */
    /* ---------------------------------------------------------------------- */

    // Read back a register we just wrote to ensure MOSI and MISO are both intact
    if (CC1101_ReadReg(dev, CC1101_REG_IOCFG2) != 0x29) {
        return HAL_ERROR;
    }

    /* ---------------------------------------------------------------------- */
    /* 4. CONTINUE CONFIGURATION                                              */
    /* ---------------------------------------------------------------------- */

    // 433.92 MHz base frequency (assuming 26MHz crystal)
    CC1101_WriteReg(dev, CC1101_REG_FREQ2,    0x10);
    CC1101_WriteReg(dev, CC1101_REG_FREQ1,    0xA7);
    CC1101_WriteReg(dev, CC1101_REG_FREQ0,    0x62);

    // GFSK, 250kBaud, Variable length packets
    CC1101_WriteReg(dev, CC1101_REG_MDMCFG4,  0x2D);
    CC1101_WriteReg(dev, CC1101_REG_MDMCFG3,  0x3B);
    CC1101_WriteReg(dev, CC1101_REG_MDMCFG2,  0x13);
    CC1101_WriteReg(dev, CC1101_REG_MDMCFG1,  0x22);
    CC1101_WriteReg(dev, CC1101_REG_MDMCFG0,  0xF8);

    // Packet configuration: Variable length, CRC enabled, Append Status
    CC1101_WriteReg(dev, CC1101_REG_PKTCTRL1, 0x04);
    CC1101_WriteReg(dev, CC1101_REG_PKTCTRL0, 0x05);
    CC1101_WriteReg(dev, CC1101_REG_PKTLEN,   0x3D); // Max length 61 bytes

    // Auto-calibrate when going from IDLE to RX/TX
    CC1101_WriteReg(dev, CC1101_REG_MCSM0,    0x18);

    dev->state = CC1101_STATE_IDLE;
    dev->dma_busy = false;

    return HAL_OK;
}

void CC1101_SetRX(cc1101_t *dev) {
    CC1101_Strobe(dev, CC1101_STROBE_SIDLE);
    CC1101_Strobe(dev, CC1101_STROBE_SFRX);   // Flush RX buffer
    CC1101_Strobe(dev, CC1101_STROBE_SRX);    // Enter RX
    dev->state = CC1101_STATE_RX;
}

void CC1101_SetTX(cc1101_t *dev) {
    CC1101_Strobe(dev, CC1101_STROBE_SIDLE);
    CC1101_Strobe(dev, CC1101_STROBE_SFTX);   // Flush TX buffer
    dev->state = CC1101_STATE_TX;
}

HAL_StatusTypeDef CC1101_Transmit_DMA(cc1101_t *dev, uint8_t *data, uint8_t len) {
    if (len > 61) return HAL_ERROR;
    if (dev->dma_busy) return HAL_BUSY;

    CC1101_SetTX(dev);

    // Format: [Burst Write Cmd] [Length Byte] [Payload...]
    dev->tx_fifo[0] = CC1101_REG_TXFIFO | CC1101_WRITE_BURST;
    dev->tx_fifo[1] = len;

    for(uint8_t i = 0; i < len; i++) {
        dev->tx_fifo[i + 2] = data[i];
    }

    dev->dma_busy = true;
    CS_Low(dev);

    // Using DMA to transfer the prepared buffer
    // According to image_6cf3dc.png, SPI3_TX is Memory to Peripheral, DMA1 Stream 5.
    HAL_StatusTypeDef res = HAL_SPI_Transmit_DMA(dev->hspi, dev->tx_fifo, len + 2);

    if (res != HAL_OK) {
        CS_High(dev);
        dev->dma_busy = false;
        return res;
    }

    // Note: STX strobe must be triggered AFTER the DMA writes to the FIFO
    // This is handled in the DMA complete callback.
    return HAL_OK;
}

HAL_StatusTypeDef CC1101_Receive_DMA(cc1101_t *dev) {
    if (dev->dma_busy) return HAL_BUSY;

    uint8_t rx_bytes = CC1101_ReadStatus(dev, CC1101_STATUS_RXBYTES) & 0x7F;

    if(rx_bytes > 0 && rx_bytes <= 64) {
        dev->rx_len = rx_bytes;

        // Setup dummy TX for clock generation during RX DMA
        dev->tx_fifo[0] = CC1101_REG_RXFIFO | CC1101_READ_BURST;
        for(uint8_t i = 1; i <= rx_bytes; i++) {
            dev->tx_fifo[i] = CC1101_STROBE_SNOP;
        }

        dev->dma_busy = true;
        CS_Low(dev);

        // SPI3_RX is Peripheral to Memory, DMA1 Stream 0.
        // We use TransmitReceive to generate the clock with TX while reading RX
        return HAL_SPI_TransmitReceive_DMA(dev->hspi, dev->tx_fifo, dev->rx_fifo, rx_bytes + 1);
    }

    return HAL_ERROR;
}

void CC1101_DMA_Complete_Callback(cc1101_t *dev) {
    if(dev->dma_busy) {
        CS_High(dev);
        dev->dma_busy = false;

        if (dev->state == CC1101_STATE_TX) {
            // Initiate the transmission now that data is in TX FIFO
            CC1101_Strobe(dev, CC1101_STROBE_STX);
        }
        else if (dev->state == CC1101_STATE_RX) {
            // Extract RSSI and LQI from appended status bytes if needed
            if (dev->rx_len >= 2) {
                // Approximate RSSI calculation
                uint8_t rssi_dec = dev->rx_fifo[dev->rx_len - 1];
                if(rssi_dec >= 128) dev->rssi = (int8_t)((rssi_dec - 256) / 2 - 74);
                else                dev->rssi = (int8_t)((rssi_dec) / 2 - 74);

                dev->lqi = dev->rx_fifo[dev->rx_len] & 0x7F;
            }
            // Flush and return to RX state for the next packet
            CC1101_SetRX(dev);
        }
    }
}

void CC1101_Interrupt_Handler(cc1101_t *dev) {
    // This is called by your EXTI (e.g. falling edge of GDO0)
    // Indicating a packet has finished transmitting or receiving
    if(dev->state == CC1101_STATE_RX) {
        // Trigger DMA to pull the data from the RX FIFO
        CC1101_Receive_DMA(dev);
    }
    else if (dev->state == CC1101_STATE_TX) {
        // Transmission finished, return to RX mode or IDLE
        CC1101_SetRX(dev);
    }
}
