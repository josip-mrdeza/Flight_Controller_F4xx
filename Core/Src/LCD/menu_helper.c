#include "LCD/menu_helper.h"

void Menu_Init(AppData_t *data) {
	data->currentState = STATE_INIT;
	data->selectedItem = 0;
}

void Menu_Draw(I2C_HandleTypeDef *hi2c, AppData_t *data, GY6500_Data_t*  imu_data, Orientation_t* orientation_data) {
	char buff[24];
	SSD1315_Clear();
	int prev_state = data->currentState;
	if(imu_data != NULL && orientation_data != NULL)
	{
		data->currentState = STATE_GYROSCOPE;
	}
	switch(data->currentState) {
		case STATE_INIT:
			SSD1315_Title("[INIT]");
			break;
		case STATE_RX_RADIO:
			SSD1315_Title("[RX RADIO]");
			break;
		case STATE_TX_RADIO:
			SSD1315_Title("[TX RADIO]");
			break;
		case STATE_GYROSCOPE:
			SSD1315_Title("[GYROSCOPE]");
			sprintf(buff, "X:%.1fdeg/%.1fm/s2", orientation_data->roll_deg, imu_data->accel_x);
			SSD1315_Line_1(buff);
			sprintf(buff, "Y:%.1fdeg/%.1fm/s2", orientation_data->pitch_deg, imu_data->accel_y);
			SSD1315_Line_2(buff);
			sprintf(buff, "Z:%.1fdeg/%.1fm/s2", imu_data->yaw, imu_data->accel_z);
			SSD1315_Line_3(buff);
			break;
	}
	data->currentState = prev_state;

	SSD1315_UpdateScreen(hi2c);
}
