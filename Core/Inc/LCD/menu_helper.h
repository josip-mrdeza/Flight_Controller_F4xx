/*
 * menu_helper.h
 *
 *  Created on: Jul 22, 2026
 *      Author: joki
 */
#ifndef __SSD1315_H__
#include "ssd1315.h"
#endif
#ifndef DRIVERS_GY6500_GY6500_H_
#include "Drivers/gy6500.h"
#endif
#ifndef INC_LCD_MENU_HELPER_H_

#define INC_LCD_MENU_HELPER_H_
typedef struct {
	I2C_HandleTypeDef *hi2c;
	AppData_t *data; GY6500_Data_t*  imu_data;
	Orientation_t* orientation_data;
	_Bool waiting_ack;
	_Bool flag_reset_transmit;
} Menu_data_t;
extern Menu_data_t menu_data;
extern _Bool display_off;
void Menu_Init(I2C_HandleTypeDef *hi2c, AppData_t *data, GY6500_Data_t*  imu_data, Orientation_t* orientation_data);

void Menu_Draw();


#endif /* INC_LCD_MENU_HELPER_H_ */
