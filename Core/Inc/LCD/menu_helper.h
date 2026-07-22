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

void Menu_Init(AppData_t *data);

void Menu_Draw(I2C_HandleTypeDef *hi2c, AppData_t *data, GY6500_Data_t*  imu_data, Orientation_t* orientation_data);


#endif /* INC_LCD_MENU_HELPER_H_ */
