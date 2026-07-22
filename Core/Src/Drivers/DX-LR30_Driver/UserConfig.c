



#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "Drivers/DX-LR30_Driver/UserConfig.h"
#include "Drivers/DX-LR30_Driver/sx126x.h"
#include "Drivers/DX-LR30_Driver/sx126x_hal.h"
#include <string.h>
#include <stdlib.h>
#include "LCD/menu_helper.h"


static volatile uint8_t g_lora_test_rxs;
static volatile uint8_t g_lora_tx_done;

volatile uint32_t lastTransmitDelay = 0;
volatile uint32_t tickTransmitStart = 0;
volatile uint32_t tickTransmitEnd = 0;
volatile uint8_t lastTransmitLength = 0;
volatile float approxDataTransferSpeed = 0;
volatile uint8_t IrqFired = 0;
sx126x_rx_buffer_status_t offset = {0};
sx126x_pkt_status_lora_t RadioPktStatus;
sx126x_irq_mask_t radioFlag = 0;

static volatile RadioOperatingModes_t OperatingMode;
void LoraOpenRXMode(uint8_t Timerout);


void SetTxHz(uint16_t HZ)
{
	sx126x_set_rf_freq(NULL,HZ * 1000000);

}




RadioOperatingModes_t sx1262GetOperatingMode(void)
{
	return OperatingMode;
}

void sx1262SetOperatingMode(RadioOperatingModes_t mode)
{
	OperatingMode = mode;
}



void RxEn(void)
{

	HAL_GPIO_WritePin(LCC68_RXEN_PORT, LCC68_RXEN_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCC68_TXEN_PORT, LCC68_TXEN_PIN, GPIO_PIN_RESET);

}
void TxEn(void)
{

	HAL_GPIO_WritePin(LCC68_RXEN_PORT, LCC68_RXEN_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCC68_TXEN_PORT, LCC68_TXEN_PIN, GPIO_PIN_SET);

}




void LoraInit(void)
{
	g_lora_test_rxs = false;
	g_lora_tx_done = true;

	/* IO复位+CS唤醒 模块*/
	sx126x_reset(NULL);
	sx126x_wakeup(NULL);

	/* 状态机设定 */
	/* 进入 STDBY_RC 待机配置模式 */
	sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC );
	sx126x_set_standby(NULL, SX126X_STANDBY_CFG_XOSC );

	/* 选择内部电压调节器模式 高效DC-DC */
	sx126x_set_reg_mode(NULL, SX126X_REG_MODE_DCDC);

	/* 内部FIFO读写地址复位 0x00 */
	sx126x_set_buffer_base_address(NULL,0x00,0x00);

	sx126x_set_pkt_type(NULL,SX126X_PKT_TYPE_LORA);
	sx126x_set_trimming_capacitor_values(NULL,0x4,0x2f);

	sx126x_mod_params_lora_t params;
	params.bw = SX126X_LORA_BW_500;
	params.sf = SX126X_LORA_SF7;
	params.cr = SX126X_LORA_CR_4_6;
	params.ldro = 0x00;
	sx126x_set_lora_mod_params(NULL, &params);

	sx126x_pkt_params_lora_t params2;
	params2.crc_is_on = 0;
	params2.invert_iq_is_on = 0;
	params2.pld_len_in_bytes = 0xff;
	params2.header_type = SX126X_LORA_PKT_EXPLICIT;
	params2.preamble_len_in_symb = LORA_PREAMBLE_LENGTH;
	sx126x_set_lora_pkt_params(NULL, &params2);


	sx126x_pa_cfg_params_t  params3;
	params3.pa_duty_cycle = 0x04;
	params3.hp_max = 0x07;
	params3.device_sel = 0x00;
	params3.pa_lut = 0x01;
	sx126x_set_pa_cfg(NULL, &params3);
	//打开dio1的中断 中断触发 SX126X_IRQ_RX_DONE | SX126X_IRQ_TX_DONE
	sx126x_set_dio_irq_params(NULL, SX126X_IRQ_RX_DONE | SX126X_IRQ_TX_DONE,SX126X_IRQ_RX_DONE | SX126X_IRQ_TX_DONE, SX126X_IRQ_NONE, SX126X_IRQ_NONE);
	sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);
	sx126x_set_tx_params(NULL,22 ,SX126X_RAMP_3400_US);
	sx126x_write_register(NULL, 0x08E7, (uint8_t[]){0x38}, 1);

	/* 设置载波频率(频点) */
	sx126x_set_rf_freq(NULL,LORA_FRE);

#if TEST
	g_lora_test_rxs = true;
	TxEn();
#else
	LoraOpenRXMode(LORA_SX126x_SYMBOL_TIMEOUT);
#endif
}

_Bool DX_LR30_Ping(void)
{
	uint8_t sync_word[2] = {0x00, 0x00};

	// Read Register 0x0740 (LoRa Sync Word, length 2 bytes)
	// Opcode for Read Register is 0x1D
	sx126x_read_register(NULL, 0x0740, sync_word, 2);

	// Default private network sync word is 0x14 0x24 (or public 0x34 0x44)
	if ((sync_word[0] == 0x14 && sync_word[1] == 0x24) ||
			(sync_word[0] == 0x34 && sync_word[1] == 0x44))
	{
		return true; // SPI IS WORKING & RADIO IS ALIVE!
	}

	return false; // SPI or Power Failure
}
void set_LoraPacketParams(uint8_t size)
{

	sx126x_pkt_params_lora_t params2;
	params2.crc_is_on = 0;
	params2.invert_iq_is_on = 0;
	params2.pld_len_in_bytes = size;
	params2.header_type = SX126X_LORA_PKT_EXPLICIT;
	params2.preamble_len_in_symb = LORA_PREAMBLE_LENGTH;
	sx126x_set_lora_pkt_params(NULL, &params2);
}


void LoraDataSend(uint8_t *data,uint8_t len)
{
	TxEn();
	set_LoraPacketParams(len);
	sx126x_write_buffer(NULL, 0x00, data, len);
	sx126x_set_tx(NULL,6000);
	g_lora_tx_done = false;
	g_lora_test_rxs = false;
	sx1262SetOperatingMode(MODE_TX);
}




void LoraOpenRXMode(uint8_t Timerout)
{
	g_lora_test_rxs = true;
	sx126x_set_rx(NULL,Timerout);
	sx1262SetOperatingMode(MODE_RX);
	RxEn();

}



//以下为接收，发送处理
void OnTxDone(void)
{    
	g_lora_tx_done = true;
	g_lora_test_rxs = true;
	tickTransmitEnd = HAL_GetTick();
	lastTransmitDelay = tickTransmitEnd - tickTransmitStart;
	approxDataTransferSpeed = lastTransmitLength / (lastTransmitDelay / 1000.0f); //B/s
	LoraOpenRXMode(LORA_SX126x_SYMBOL_TIMEOUT);
	HAL_TIM_Base_Start_IT(&htim3);
	menu_data.data->currentState = STATE_TX_RADIO;
	//Menu_Draw();
	char buff[24];
	sprintf(buff, "DtR: %.2f B/s", approxDataTransferSpeed);
	SSD1315_Line_1(buff);
	sprintf(buff, "Transferred: %d B", lastTransmitLength);
	SSD1315_Line_2(buff);
	sprintf(buff, "Dt: %d ms", lastTransmitDelay);
	SSD1315_Line_3(buff);
	SSD1315_UpdateScreen(menu_data.hi2c);
}

void OnRxDone(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr)
{
	g_lora_tx_done = true;
	g_lora_test_rxs = true;
	LoraOpenRXMode(LORA_SX126x_SYMBOL_TIMEOUT);
	menu_data.data->currentState = STATE_RX_RADIO;
	Menu_Draw();
	SSD1315_Line_1("Received data: ");
	char buff[24];
	memcpy(buff, payload, 24);
	SSD1315_Line_2(buff);
	memset(buff, 0, 24);
	sprintf(buff, "RSSI/SNR: %d/%d", rssi, snr);
	SSD1315_Line_3(buff);
	SSD1315_UpdateScreen(menu_data.hi2c);
	LoraOpenRXMode(LORA_SX126x_SYMBOL_TIMEOUT);
}

void RxError(void)
{

	g_lora_tx_done = true;
	g_lora_test_rxs = true;




}

void CadDone ( bool channelActivityDetected )
{

	g_lora_test_rxs = true;
	g_lora_tx_done = true;


}

void RxTimeout(void)
{
	g_lora_tx_done = true;
	g_lora_test_rxs = true;
	LoraOpenRXMode(LORA_SX126x_SYMBOL_TIMEOUT);
}

void TxTimeout(void)
{
	g_lora_tx_done = true;
	g_lora_test_rxs = true;
	LoraOpenRXMode(LORA_SX126x_SYMBOL_TIMEOUT);
}


uint8_t radioRxbuff[255] = {0};
void Hz_set(char *data,uint8_t len)
{
	uint32_t num = strtol(data, NULL, 10);

	SetTxHz(num);
}

void Data_Processing(void)
{

	if(g_lora_tx_done == true && g_lora_test_rxs == true)
	{
#if !TEST
		char buff[255];
		//snprintf(buff, sizeof(buff), "Controller feedback to plane brain.");
		snprintf(buff, sizeof(buff), "Plane brain feedback to controller interface!");
		tickTransmitStart = HAL_GetTick(); //ms
		uint8_t len = (uint8_t) strlen(buff);
		lastTransmitLength = len;
		LoraDataSend((uint8_t *)buff, len);
#else
		uint8_t mydata[SIZE_DATA] = {0};
		uint8_t len = queueDequeue(pUart1RxQueue, &mydata);
		Hz_set((char *)mydata,len);
#endif
	}
}





void DX_Lora_RadioIrqProcess(void)
{

	if(IrqFired == true)
	{
		__disable_irq();
		IrqFired = false;
		__enable_irq();

		if( ( radioFlag & SX126X_IRQ_TX_DONE ) == SX126X_IRQ_TX_DONE )
		{
			sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC );
			OnTxDone();
		}
		if( ( radioFlag & SX126X_IRQ_RX_DONE ) ==  SX126X_IRQ_RX_DONE )
		{
			sx126x_set_standby(NULL,SX126X_STANDBY_CFG_RC );
			sx126x_get_rx_buffer_status(NULL, &offset);
			sx126x_read_buffer(NULL, offset.buffer_start_pointer, radioRxbuff, offset.pld_len_in_bytes);
			sx126x_get_lora_pkt_status(NULL, &RadioPktStatus);
			OnRxDone(&radioRxbuff[0],  offset.pld_len_in_bytes, RadioPktStatus.rssi_pkt_in_dbm + RadioPktStatus.snr_pkt_in_db, RadioPktStatus.snr_pkt_in_db);
			memset(radioRxbuff,0,255);
		}

		if( ( radioFlag &  SX126X_IRQ_CRC_ERROR ) ==  SX126X_IRQ_CRC_ERROR )
		{
			sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC );
			RxError();
		}

		if( ( radioFlag &  SX126X_IRQ_CAD_DONE ) ==  SX126X_IRQ_CAD_DONE )
		{
			sx126x_set_standby(NULL,SX126X_STANDBY_CFG_RC );
			CadDone((radioFlag & SX126X_IRQ_CAD_DETECTED) == SX126X_IRQ_CAD_DETECTED);
		}

		if((radioFlag & SX126X_IRQ_TIMEOUT) == SX126X_IRQ_TIMEOUT)
		{
			sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC );
			if( sx1262GetOperatingMode( ) == MODE_TX )
			{
				TxTimeout();

			}else if( sx1262GetOperatingMode( ) == MODE_RX )
			{
				RxTimeout();
			}
		}

		if( ( radioFlag & SX126X_IRQ_PREAMBLE_DETECTED ) == SX126X_IRQ_PREAMBLE_DETECTED )
		{
			//__NOP( );
		}

		if( ( radioFlag & SX126X_IRQ_SYNC_WORD_VALID ) == SX126X_IRQ_SYNC_WORD_VALID )
		{
			//__NOP( );
		}

		if( ( radioFlag & SX126X_IRQ_HEADER_VALID ) == SX126X_IRQ_HEADER_VALID )
		{
			//__NOP( );
		}

		if( ( radioFlag & SX126X_IRQ_HEADER_ERROR ) == SX126X_IRQ_HEADER_ERROR )
		{
			RxTimeout();
		}


	}

}
