/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "LCD/ssd1315.h"
#include "LCD/menu_helper.h"
#include "Drivers/gy6500.h"
#include "Drivers/gy273.h"
#include "Drivers/pca9685.h"
#include "Drivers/cc1101.h"
#include "Drivers/i2c_helper.h"
#include "Drivers/DX-LR30_Driver/sx126x.h"
#include "Drivers/DX-LR30_Driver/driver_DIO1.h"
#include "Drivers/DX-LR30_Driver/UserConfig.h"
#include "Drivers/gy_gps6mv2.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct {
    bool                (*is_controller)(void);
    bool                (*is_plane)(void);

    GY6500_Data_t       imu_data;
    GY6500_Calib_t      imu_calib;
    GY273_RawData_t     mag_data;
    GY273_Calib_t       mag_calib;
    Orientation_t       orientation;
    float               heading_2d;
    float               heading_3d;
    float               compass_heading;
    uint32_t            delta_time_ms;

    cc1101_t            cc1101;
    GPS_HandleTypeDef   gps;
    AppData_t           gui_data;

    volatile bool       flag_gyro_update;
    volatile bool       flag_transmit;
    volatile bool       packet_received;
    volatile bool       flag_waiting_ack;
} AppContext_t;

extern AppContext_t g_app;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#ifndef container_of
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
#endif
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void init_all(void);
bool is_controller(void);
bool is_plane(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define onboard_LED_AL_Pin GPIO_PIN_2
#define onboard_LED_AL_GPIO_Port GPIOB
#define CS_NSS_Pin GPIO_PIN_2
#define CS_NSS_GPIO_Port GPIOD
#define GDO0_EXTI_Pin GPIO_PIN_3
#define GDO0_EXTI_GPIO_Port GPIOB
#define GDO0_EXTI_EXTI_IRQn EXTI3_IRQn
#define GDO2_EXTI_Pin GPIO_PIN_4
#define GDO2_EXTI_GPIO_Port GPIOB
#define GDO2_EXTI_EXTI_IRQn EXTI4_IRQn
#define NRST_Pin GPIO_PIN_5
#define NRST_GPIO_Port GPIOB
#define LR30_BUSY_Pin GPIO_PIN_6
#define LR30_BUSY_GPIO_Port GPIOB
#define LR30_RXEN_Pin GPIO_PIN_7
#define LR30_RXEN_GPIO_Port GPIOB
#define LR30_TXEN_Pin GPIO_PIN_8
#define LR30_TXEN_GPIO_Port GPIOB
#define DEF_SWITCH_C_P_Pin GPIO_PIN_9
#define DEF_SWITCH_C_P_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
