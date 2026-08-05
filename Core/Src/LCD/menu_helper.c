#include "LCD/menu_helper.h"
Menu_data_t menu_data;
_Bool display_off;
void Menu_Init(I2C_HandleTypeDef *hi2c, AppData_t *data, GY6500_Data_t*  imu_data, Orientation_t* orientation_data) {
	data->currentState = STATE_INIT;
	data->selectedItem = 0;
	menu_data.data = data;
	menu_data.hi2c = hi2c;
	menu_data.imu_data = imu_data;
	menu_data.orientation_data = orientation_data;
}

void Menu_Draw() {
	if(display_off)
	{
		return;
	}
	char buff[24];
	SSD1315_Clear();
	int prev_state = menu_data.data->currentState;
	switch(menu_data.data->currentState) {
		case STATE_INIT:
			SSD1315_Title("[INIT]");
			break;
		case STATE_RX_RADIO:
			sprintf(buff, "[RX RADIO] - %s", *menu_data.data->is_controller_ptr ? "Controller" : "Plane");
			SSD1315_Title(buff);
			break;
		case STATE_TX_RADIO:
			sprintf(buff, "[TX RADIO] - %s", *menu_data.data->is_controller_ptr ? "Controller" : "Plane");
			SSD1315_Title(buff);
			break;
		case STATE_GYROSCOPE:
			sprintf(buff, "[GYROSCOPE] - %s", *menu_data.data->is_controller_ptr ? "Controller" : "Plane");
			SSD1315_Title(buff);
			sprintf(buff, "X:%.1fdeg/%.1fm/s2", menu_data.orientation_data->roll_deg, menu_data.imu_data->accel_x);
			SSD1315_Line_1(buff);
			sprintf(buff, "Y:%.1fdeg/%.1fm/s2", menu_data.orientation_data->pitch_deg, menu_data.imu_data->accel_y);
			SSD1315_Line_2(buff);
			sprintf(buff, "Z:%.1fdeg/%.1fm/s2", menu_data.imu_data->yaw, menu_data.imu_data->accel_z);
			SSD1315_Line_3(buff);
			break;
	}
	menu_data.data->currentState = prev_state;

	SSD1315_UpdateScreen(menu_data.hi2c);
}
