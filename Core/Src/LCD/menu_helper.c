#include "LCD/menu_helper.h"
#include "main.h"
#include "Drivers/DX-LR30_Driver/adr.h"
#include <stdio.h>
#include <string.h>
Menu_data_t menu_data;
_Bool display_off;
HAL_StatusTypeDef Menu_Init(I2C_HandleTypeDef *hi2c, AppData_t *data, GY6500_Data_t*  imu_data, Orientation_t* orientation_data) {
	if (hi2c == NULL || data == NULL || imu_data == NULL || orientation_data == NULL) {
		return HAL_ERROR;
	}
	data->currentState = STATE_INIT;
	data->selectedItem = 0;
	menu_data.data = data;
	menu_data.hi2c = hi2c;
	menu_data.imu_data = imu_data;
	menu_data.orientation_data = orientation_data;
	menu_data.waiting_ack = 0;
	menu_data.flag_reset_transmit = 0;
	Menu_Draw();
	return HAL_OK;
}

void Menu_Draw() {
	if(display_off)
	{
		return;
	}
	char buff[32];
	SSD1315_Clear();
	AppContext_t *app = container_of(menu_data.data, AppContext_t, gui_data);
	int prev_state = menu_data.data->currentState;
	switch(menu_data.data->currentState) {
		case STATE_INIT:
			SSD1315_Title("[INIT]");
			break;
		case STATE_RX_RADIO:
			snprintf(buff, sizeof(buff), "%.0fDR/%.0fSF - %s", (float)g_adr.current_tier, (float)ADR_GetSF(g_adr.current_tier), (app->is_controller && app->is_controller()) ? "Ctrl" : "Plane");
			SSD1315_Title(buff);
			break;
		case STATE_TX_RADIO:
			snprintf(buff, sizeof(buff), "%.0fDR/%.0fSF - %s", (float)g_adr.current_tier, (float)ADR_GetSF(g_adr.current_tier), (app->is_controller && app->is_controller()) ? "Ctrl" : "Plane");
			SSD1315_Title(buff);
			break;
		case STATE_GYROSCOPE:
			snprintf(buff, sizeof(buff), "[GYROSCOPE] - %s", (app->is_controller && app->is_controller()) ? "Controller" : "Plane");
			SSD1315_Title(buff);
			snprintf(buff, sizeof(buff), "X:%.1fdeg/%.1fm/s2", menu_data.orientation_data->roll_deg, menu_data.imu_data->accel_x);
			SSD1315_Line_1(buff);
			snprintf(buff, sizeof(buff), "Y:%.1fdeg/%.1fm/s2", menu_data.orientation_data->pitch_deg, menu_data.imu_data->accel_y);
			SSD1315_Line_2(buff);
			snprintf(buff, sizeof(buff), "Z:%.1fdeg/%.1fm/s2", menu_data.imu_data->yaw, menu_data.imu_data->accel_z);
			SSD1315_Line_3(buff);
			break;
	}
	menu_data.data->currentState = prev_state;

	SSD1315_UpdateScreen(menu_data.hi2c);
}
