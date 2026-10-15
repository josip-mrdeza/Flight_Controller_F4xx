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
#include "Drivers/DX-LR30_Driver/adr.h"
#include "Drivers/DX-LR30_Driver/lora_telemetry.h"
#include <string.h>
#include <stdlib.h>
#include "LCD/menu_helper.h"
#include "usbd_cdc_if.h"
#include "i2c.h"

static volatile uint8_t g_lora_test_rxs;
static volatile uint8_t g_lora_tx_done;

volatile uint32_t lastTransmitDelay = 0;
volatile uint32_t tickTransmitStart = 0;
volatile uint32_t tickTransmitEnd = 0;
volatile uint8_t lastTransmitLength = 0;
volatile uint32_t tick_roundTripPing = 0;
volatile float approxDataTransferSpeed = 0;
RadioMetricsStorage_t g_radio_metrics = {0};
volatile uint8_t IrqFired = 0;
sx126x_rx_buffer_status_t offset = {0};
sx126x_pkt_status_lora_t RadioPktStatus;
sx126x_irq_mask_t radioFlag = 0;

static volatile RadioOperatingModes_t OperatingMode;
volatile uint32_t LORA_SX126x_SYMBOL_TIMEOUT = 0;
void LoraOpenRXMode(uint32_t Timerout);

void SetTxHz(float mhz)
{
	sx126x_set_rf_freq(NULL, (uint16_t) (mhz * 1000000));
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

uint32_t SX126x_CalcSymbolTimeout(void)
{
    uint32_t bw_hz = ADR_GetBW_Hz(g_adr.current_tier);
    uint8_t sf = ADR_GetSF(g_adr.current_tier);
    float tSym = (float)(1 << sf) / (float)bw_hz;
    (void)tSym;
    float timeoutSymbols = LORA_PREAMBLE_LENGTH + 4.25f;
    return (uint32_t)(timeoutSymbols);
}

uint32_t SX126x_TimeoutMs_To_Symbols(uint32_t timeout_ms)
{
    uint32_t bw_hz = ADR_GetBW_Hz(g_adr.current_tier);
    uint8_t sf = ADR_GetSF(g_adr.current_tier);
    float tSym_ms = ((float)(1 << sf) / (float)bw_hz) * 1000.0f;
    uint32_t symbols = (uint32_t)(timeout_ms / tSym_ms);
    if(symbols < 1) symbols = 1;
    return symbols;
}

uint32_t SX126x_BW_Hz(uint8_t bw)
{
    switch(bw)
    {
        case SX126X_LORA_BW_007: return 7800;
        case SX126X_LORA_BW_010: return 10400;
        case SX126X_LORA_BW_015: return 15600;
        case SX126X_LORA_BW_020: return 20800;
        case SX126X_LORA_BW_031: return 31200;
        case SX126X_LORA_BW_041: return 41700;
        case SX126X_LORA_BW_062: return 62500;
        case SX126X_LORA_BW_125: return 125000;
        case SX126X_LORA_BW_250: return 250000;
        case SX126X_LORA_BW_500: return 500000;
        default: return 125000;
    }
}

void LoraInit(void)
{
	g_lora_test_rxs = false;
	g_lora_tx_done = true;

	sx126x_reset(NULL);
	sx126x_wakeup(NULL);

	sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
	sx126x_set_standby(NULL, SX126X_STANDBY_CFG_XOSC);

	sx126x_set_reg_mode(NULL, SX126X_REG_MODE_DCDC);
	sx126x_set_buffer_base_address(NULL, 0x00, 0x00);
	sx126x_set_pkt_type(NULL, SX126X_PKT_TYPE_LORA);
	sx126x_set_trimming_capacitor_values(NULL, 0x4, 0x2f);

	ADR_Init();
	ADR_ApplyTier(g_adr.current_tier);

	sx126x_pa_cfg_params_t params3;
	params3.pa_duty_cycle = 0x04;
	params3.hp_max = 0x07;
	params3.device_sel = 0x00;
	params3.pa_lut = 0x01;
	sx126x_set_pa_cfg(NULL, &params3);

	sx126x_set_dio_irq_params(NULL, SX126X_IRQ_RX_DONE | SX126X_IRQ_TX_DONE, SX126X_IRQ_RX_DONE | SX126X_IRQ_TX_DONE, SX126X_IRQ_NONE, SX126X_IRQ_NONE);
	sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);
	sx126x_set_tx_params(NULL, 22, SX126X_RAMP_1700_US);
	sx126x_write_register(NULL, 0x08E7, (uint8_t[]){0x38}, 1);

	sx126x_set_rf_freq(NULL, LORA_FRE);
	sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);

#if TEST
	g_lora_test_rxs = true;
	TxEn();
#else
	LoraOpenRXMode(SX126X_RX_CONTINUOUS);
#endif
}

_Bool DX_LR30_Ping(void)
{
	uint8_t sync_word[2] = {0x00, 0x00};
	sx126x_read_register(NULL, 0x0740, sync_word, 2);

	if ((sync_word[0] == 0x14 && sync_word[1] == 0x24) ||
			(sync_word[0] == 0x34 && sync_word[1] == 0x44))
	{
		return true;
	}

	return false;
}

HAL_StatusTypeDef DX_LR30_Init(void)
{
	char buff[32];
	Menu_Draw();
	snprintf(buff, sizeof(buff), "[DX-LR30-900MHz]");
	SSD1315_Title(buff);
	snprintf(buff, sizeof(buff), "Init DX-LR30: ...");
	SSD1315_Line_1(buff);
	SSD1315_UpdateScreen(&hi2c3);

	LoraInit();
	HAL_Delay(25);
	_Bool status = DX_LR30_Ping();

	snprintf(buff, sizeof(buff), "Init DX-LR30: %s", status ? "Ok" : "Fail");
	SSD1315_Line_1(buff);
	SSD1315_UpdateScreen(&hi2c3);

	return status ? HAL_OK : HAL_ERROR;
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

void LoraDataSend(uint8_t *data, uint8_t len)
{
	TxEn();
	set_LoraPacketParams(len);
	sx126x_write_buffer(NULL, 0x00, data, len);
	sx126x_set_tx(NULL, 6000);
	g_lora_tx_done = false;
	g_lora_test_rxs = false;
	sx1262SetOperatingMode(MODE_TX);
}

void LoraOpenRXMode(uint32_t Timerout)
{
	g_lora_test_rxs = true;

	sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
	sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);
	set_LoraPacketParams(255);

	RxEn();
	if (Timerout == SX126X_RX_CONTINUOUS || Timerout == 0 || Timerout == 0xFFFFFF) {
		sx126x_set_rx_with_timeout_in_rtc_step(NULL, SX126X_RX_CONTINUOUS);
	} else {
		sx126x_set_rx(NULL, Timerout);
	}
	sx1262SetOperatingMode(MODE_RX);
}

void OnTxDone(void)
{    
	g_lora_tx_done = true;
	g_lora_test_rxs = true;
	tickTransmitEnd = HAL_GetTick();
	lastTransmitDelay = (uint32_t)((float) tickTransmitEnd - (float) tickTransmitStart - 1.5f);
	g_adr.last_tx_time_ts_ms = lastTransmitDelay;
	approxDataTransferSpeed = lastTransmitLength / (lastTransmitDelay / 1000.0f);

	if (is_plane()) {
		if (g_adr.tier_changed) {
			ADR_ApplyTier(g_adr.target_tier);
		} else {
			LoraOpenRXMode(SX126X_RX_CONTINUOUS);
		}
		menu_data.waiting_ack = 0;
	} else {
		LoraOpenRXMode(SX126X_RX_CONTINUOUS);
		menu_data.waiting_ack = 1;
		htim4.Instance->ARR = ADR_GetAckTimeoutMs(g_adr.current_tier);
		htim4.Instance->CNT = 0;
		HAL_TIM_Base_Start_IT(&htim4);
	}

	menu_data.data->currentState = STATE_TX_RADIO;
	UpdateMenuADR(STATE_TX_RADIO, 0, 0);
}

void UpdateMenuADR(uint8_t state, int16_t rssi, int8_t snr)
{
	if (display_off)
	{
		return;
	}

	static uint32_t last_draw_tick = 0;
	uint32_t now = HAL_GetTick();
	if ((now - last_draw_tick) < 100)
	{
		return;
	}
	last_draw_tick = now;

	static int last_menu_state = -1;
	bool force_redraw = false;

	if (menu_data.data != NULL && menu_data.data->currentState != last_menu_state)
	{
		last_menu_state = menu_data.data->currentState;
		force_redraw = true;
	}

	if (!g_radio_metrics.initialized)
	{
		force_redraw = true;
		g_radio_metrics.initialized = true;
	}

	g_radio_metrics.current.tier = g_adr.current_tier;
	g_radio_metrics.current.is_ctrl = is_controller() ? 1 : 0;
	g_radio_metrics.current.rssi = (state == STATE_RX_RADIO) ? rssi : g_adr.last_rssi;
	g_radio_metrics.current.snr = (state == STATE_RX_RADIO) ? snr : g_adr.last_snr;
	g_radio_metrics.current.has_metrics = g_adr.has_metrics || (state == STATE_RX_RADIO);
	g_radio_metrics.current.ping_ms = tick_roundTripPing;
	g_radio_metrics.current.speed_bps = (uint32_t)approxDataTransferSpeed;

	char buff[32];

	if (force_redraw ||
	    g_radio_metrics.current.tier != g_radio_metrics.last.tier ||
	    g_radio_metrics.current.is_ctrl != g_radio_metrics.last.is_ctrl || true)
	{
		SSD1315_ClearLine(0);
		snprintf(buff, sizeof(buff), "%s - TTS:%ums",
		         g_radio_metrics.current.is_ctrl ? "Ctrl" : "Plane",
		        		 (unsigned int) g_adr.last_tx_time_ts_ms);
		SSD1315_Title(buff);
		SSD1315_UpdateLine(menu_data.hi2c, 0);
	}

	if (force_redraw ||
	    g_radio_metrics.current.tier != g_radio_metrics.last.tier)
	{
		SSD1315_ClearLine(1);
		snprintf(buff, sizeof(buff), "SF/BW: SF%u/%ukHz",
		         (unsigned int)ADR_GetSF(g_radio_metrics.current.tier),
		         (unsigned int)ADR_GetBW_kHz(g_radio_metrics.current.tier));
		SSD1315_Line_1(buff);
		SSD1315_UpdateLine(menu_data.hi2c, 1);
	}

	if (force_redraw ||
	    g_radio_metrics.current.rssi != g_radio_metrics.last.rssi ||
	    g_radio_metrics.current.snr != g_radio_metrics.last.snr ||
	    g_radio_metrics.current.has_metrics != g_radio_metrics.last.has_metrics)
	{
		SSD1315_ClearLine(2);
		if (g_radio_metrics.current.has_metrics)
		{
			snprintf(buff, sizeof(buff), "RSSI/SNR: %ddBm/%ddB",
			         (int)g_radio_metrics.current.rssi,
			         (int)g_radio_metrics.current.snr);
			if (strlen(buff) > 21)
			{
				snprintf(buff, sizeof(buff), "RS/SN: %ddBm/%ddB",
				         (int)g_radio_metrics.current.rssi,
				         (int)g_radio_metrics.current.snr);
			}
		}
		else
		{
			snprintf(buff, sizeof(buff), "RSSI/SNR: None");
		}
		SSD1315_Line_2(buff);
		SSD1315_UpdateLine(menu_data.hi2c, 2);
	}

	if (force_redraw ||
	    g_radio_metrics.current.ping_ms != g_radio_metrics.last.ping_ms ||
	    g_radio_metrics.current.speed_bps != g_radio_metrics.last.speed_bps)
	{
		SSD1315_ClearLine(3);
		if (g_radio_metrics.current.speed_bps > 1100)
		{
			snprintf(buff, sizeof(buff), "Ping:%lums DtR:%.2fkB/s",
			         (unsigned long)g_radio_metrics.current.ping_ms,
			         (double)g_radio_metrics.current.speed_bps / 1000.0f);
		}
		else
		{
			snprintf(buff, sizeof(buff), "Ping:%lums DtR:%lu B/s",
			         (unsigned long)g_radio_metrics.current.ping_ms,
			         (unsigned long)g_radio_metrics.current.speed_bps);
		}
		SSD1315_Line_3(buff);
		SSD1315_UpdateLine(menu_data.hi2c, 3);
	}

	g_radio_metrics.last = g_radio_metrics.current;
}

void OnRxDone(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr)
{
	g_lora_tx_done = true;
	g_lora_test_rxs = true;

	if (is_controller()) {
		HAL_TIM_Base_Stop_IT(&htim4);
		htim4.Instance->CNT = 0;
		menu_data.waiting_ack = 0;
	}

	uint8_t remote_dr = Lora_ExtractRemoteDataRate(payload, size, g_adr.current_tier);
	ADR_ProcessRxMetrics(rssi, snr, remote_dr);

	if (is_controller()) {
		HAL_TIM_Base_Start_IT(&htim3);
	} else {
		Data_Processing();
	}

	menu_data.data->currentState = STATE_RX_RADIO;
	UpdateMenuADR(STATE_RX_RADIO, rssi, snr);

	char otg_buff[512];
	uint16_t otg_len = Lora_StructureRxReport(otg_buff, sizeof(otg_buff), payload, size,
	                                         rssi, snr, g_adr.current_tier, &g_app);
	if (otg_len > 0) {
		CDC_Transmit_FS((uint8_t*)otg_buff, otg_len);
	}
}

void RxError(void)
{
	g_lora_tx_done = true;
	g_lora_test_rxs = true;
}

void CadDone(bool channelActivityDetected)
{
	g_lora_test_rxs = true;
	g_lora_tx_done = true;
}

void RxTimeout(void)
{
	g_lora_tx_done = true;
	g_lora_test_rxs = true;
	menu_data.waiting_ack = false;
	ADR_OnTimeout();
	LoraOpenRXMode(SX126X_RX_CONTINUOUS);
}

void TxTimeout(void)
{
	g_lora_tx_done = true;
	g_lora_test_rxs = true;
	menu_data.waiting_ack = false;
	ADR_OnTimeout();
	LoraOpenRXMode(SX126X_RX_CONTINUOUS);
}

uint8_t radioRxbuff[255] = {0};
void Hz_set(char *data, uint8_t len)
{
	uint32_t num = strtol(data, NULL, 10);
	SetTxHz(num);
}

void Data_Processing(void)
{
	if(g_lora_tx_done == true && g_lora_test_rxs == true)
	{
#if !TEST
		AppContext_t *app = container_of(menu_data.data, AppContext_t, gui_data);
		char buff[256];
		uint16_t len = 0;

		if(app->is_controller && app->is_controller())
		{
			len = Lora_StructureControllerPacket(buff, sizeof(buff), g_adr.target_tier, app);
			CDC_Transmit_FS((uint8_t*)buff, len);
		}
		else
		{
			len = Lora_StructurePlanePacket(buff, sizeof(buff), g_adr.target_tier, app);
		}

		if (len > 0) {
			tickTransmitStart = HAL_GetTick();
			lastTransmitLength = (uint8_t)len;
			LoraDataSend((uint8_t *)buff, (uint8_t)len);
		}
#else
		uint8_t mydata[SIZE_DATA] = {0};
		uint8_t len = queueDequeue(pUart1RxQueue, &mydata);
		Hz_set((char *)mydata, len);
#endif
	}
}

void DX_Lora_RadioIrqProcess(void)
{
	if(IrqFired || (HAL_GPIO_ReadPin(LCC68_DIO1_PORT, LCC68_DIO1_PIN) == GPIO_PIN_SET))
	{
		__disable_irq();
		IrqFired = false;
		__enable_irq();

		sx126x_get_irq_status(NULL, &radioFlag);
		sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);

		if((radioFlag & SX126X_IRQ_TX_DONE) == SX126X_IRQ_TX_DONE)
		{
			sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
			OnTxDone();
		}
		if((radioFlag & SX126X_IRQ_RX_DONE) == SX126X_IRQ_RX_DONE)
		{
			menu_data.waiting_ack = 0;
			tick_roundTripPing = HAL_GetTick() - tickTransmitStart;
			sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
			sx126x_get_rx_buffer_status(NULL, &offset);
			sx126x_read_buffer(NULL, offset.buffer_start_pointer, radioRxbuff, offset.pld_len_in_bytes);
			sx126x_get_lora_pkt_status(NULL, &RadioPktStatus);
			OnRxDone(&radioRxbuff[0], offset.pld_len_in_bytes, RadioPktStatus.rssi_pkt_in_dbm + RadioPktStatus.snr_pkt_in_db, RadioPktStatus.snr_pkt_in_db);
			memset(radioRxbuff, 0, 255);
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);
		}

		if((radioFlag & SX126X_IRQ_CRC_ERROR) == SX126X_IRQ_CRC_ERROR)
		{
			menu_data.waiting_ack = 0;
			sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
			RxError();
		}

		if((radioFlag & SX126X_IRQ_CAD_DONE) == SX126X_IRQ_CAD_DONE)
		{
			sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
			CadDone((radioFlag & SX126X_IRQ_CAD_DETECTED) == SX126X_IRQ_CAD_DETECTED);
		}

		if((radioFlag & SX126X_IRQ_TIMEOUT) == SX126X_IRQ_TIMEOUT)
		{
			menu_data.waiting_ack = 0;
			sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
			if(sx1262GetOperatingMode() == MODE_TX)
			{
				TxTimeout();
			}
			else if(sx1262GetOperatingMode() == MODE_RX)
			{
				RxTimeout();
			}
		}

		if((radioFlag & SX126X_IRQ_HEADER_ERROR) ||
		   (radioFlag & SX126X_IRQ_CRC_ERROR)    ||
		   (radioFlag & SX126X_IRQ_TIMEOUT))
		{
			RxError();
			menu_data.waiting_ack = 0;
			LoraOpenRXMode(SX126X_RX_CONTINUOUS);
		}
	}
}

void Radio_process(void)
{
	if (is_plane()) {
		uint32_t now = HAL_GetTick();
		uint32_t timeout_ms = (uint32_t)ADR_GetAckTimeoutMs(g_adr.current_tier)*5;
		if (g_adr.last_packet_tick == 0)
		{
			g_adr.last_packet_tick = now;
		}
		else if ((now - g_adr.last_packet_tick) > timeout_ms)
		{
			ADR_OnTimeout();
			LoraOpenRXMode(SX126X_RX_CONTINUOUS);
			g_adr.last_packet_tick = now;
		}
	}
	if (menu_data.flag_reset_transmit) {
		menu_data.flag_reset_transmit = 0;
		if (is_controller()) {
			g_app.flag_transmit = true;
			menu_data.waiting_ack = 0;
			ADR_OnTimeout();
		}
	}
	if (is_controller() && g_app.flag_transmit && !menu_data.waiting_ack) {
		g_app.flag_transmit = false;
		HAL_TIM_Base_Stop_IT(&htim3);
		Data_Processing();
	}
	DX_Lora_RadioIrqProcess();
}
