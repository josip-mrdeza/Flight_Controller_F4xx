/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
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

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
AppContext_t g_app = {
		.is_controller    = is_controller,
		.is_plane         = is_plane,
		.imu_data         = {0},
		.imu_calib        = {0},
		.mag_data         = {0},
		.mag_calib        = {
				.x_offset      = 6492.0f,
				.y_offset      = -6346.0f,
				.z_offset      = 16268.0f,
				.is_calibrated = 1
		},
		.orientation      = {0},
		.heading_2d       = 0.0f,
		.heading_3d       = 0.0f,
		.compass_heading  = 0.0f,
		.delta_time_ms    = 0,
		.gui_data         = {0},
		.flag_gyro_update = false,
		.flag_transmit    = false,
		.packet_received  = false,
		.flag_waiting_ack = false
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
bool is_controller(void)
{
	return HAL_GPIO_ReadPin(DEF_SWITCH_C_P_GPIO_Port, DEF_SWITCH_C_P_Pin) == GPIO_PIN_SET;
}

bool is_plane(void)
{
	return !is_controller();
}

void init_all(void)
{
	SSD1315_Init(&hi2c3);
	Menu_Init(&hi2c3, &g_app.gui_data, &g_app.imu_data, &g_app.orientation);

	GY6500_Init(&hi2c3);
	GY273_Init(&hi2c3);
	GY6500_Calibrate(&hi2c3, &g_app.imu_calib, 50);
	PCA9685_Init(&hi2c3, 50.0f);

	HAL_TIM_Base_Start_IT(&htim2);
	if (g_app.is_controller()) {
		HAL_TIM_Base_Start_IT(&htim3);
	}

	CC1101_InitDevice(&g_app.cc1101, &hspi3);
	DX_LR30_Init();
	GPS_Init(&g_app.gps, &huart1);
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_I2C3_Init();
	MX_USB_DEVICE_Init();
	MX_TIM2_Init();
	MX_SPI3_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */
	init_all();
	MX_USB_DEVICE_Init();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	IrqFired = false;
	radioFlag = 0x00;
	display_off = 0;
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		Gyro_update_data();
		gps_update_dma();
		Radio_process();
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM2)
	{
		g_app.flag_gyro_update = true;
	}
	else if (htim->Instance == TIM3)
	{
		g_app.flag_transmit = true;
	}
	else if (htim->Instance == TIM4)
	{
		menu_data.flag_reset_transmit = 1;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	/* Check if the interrupt came from the CC1101 GDO0 pin */
	if (g_app.cc1101.initok && GPIO_Pin == g_app.cc1101.gdo0_pin) {
		// This function will automatically trigger the DMA RX if in CC1101_STATE_RX
		CC1101_Interrupt_Handler(&g_app.cc1101);
	}
	else if (GPIO_Pin == LORA_DIO1_PIN)
	{
		IrqFired = true;
	}
}

/**
 * @brief Tx Transfer completed callback.
 * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
 *               the configuration information for SPI module.
 * @retval None
 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	/* Check if this is the SPI instance connected to the CC1101 */
	if (g_app.cc1101.hspi != NULL && hspi->Instance == g_app.cc1101.hspi->Instance) {
		// Pulls CS high, triggers STX strobe
		CC1101_DMA_Complete_Callback(&g_app.cc1101);
	}
}

/**
 * @brief Tx and Rx Transfer completed callback.
 * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
 *               the configuration information for SPI module.
 * @retval None
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	/* Check if this is the SPI instance connected to the CC1101 */
	if (g_app.cc1101.hspi != NULL && hspi->Instance == g_app.cc1101.hspi->Instance) {
		// Pulls CS high, extracts RSSI/LQI, puts module back into RX
		CC1101_DMA_Complete_Callback(&g_app.cc1101);

		// Notify the main loop that a packet is ready in the rx_fifo
		if (g_app.cc1101.state == CC1101_STATE_RX) {
			g_app.packet_received = true;
		}
	}
}

/**
 * @brief SPI error callback.
 * @param  hspi pointer to a SPI_HandleTypeDef structure
 * @retval None
 */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	if (g_app.cc1101.hspi != NULL && hspi->Instance == g_app.cc1101.hspi->Instance) {
		// Handle DMA/SPI errors (e.g., reset CS pin, clear busy flag)
		HAL_GPIO_WritePin(g_app.cc1101.cs_port, g_app.cc1101.cs_pin, GPIO_PIN_SET);
		g_app.cc1101.dma_busy = false;

		// Re-initialize or reset state to ensure system recovers
		CC1101_SetRX(&g_app.cc1101);
	}
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
