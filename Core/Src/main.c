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
AppData_t guiData;
GY6500_Data_t  imu_data;
GY6500_Calib_t imu_calib = {0};

GY273_RawData_t   mag_data;
GY273_Calib_t mag_calib = {
		.x_offset=6492,
		.y_offset=-6346,
		.z_offset=16268
};
cc1101_t cc1101;
float          compass_heading = 0.0f;
uint8_t flag_gyro_update;
uint8_t ms = 0;
char buff[24];
float heading_3d;
float heading_2d;
Orientation_t ori;
volatile _Bool packet_received = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void init_all()
{
	Menu_Init(&guiData);
	SSD1315_Init(&hi2c3);
	Menu_Draw(&hi2c3, &guiData, NULL, NULL);
	sprintf(buff, "Initializing...");
	SSD1315_Line_1(buff);
	SSD1315_UpdateScreen(&hi2c3);
	if (GY6500_Init(&hi2c3) != HAL_OK) {
		sprintf(buff, "Could not init GY6500");
		SSD1315_Line_2(buff);
		// Handle GY-6500 init failure
	}
	else
	{
		sprintf(buff, "Init GY6500: OK");
		SSD1315_Line_2(buff);
	}

	if (GY273_Init(&hi2c3) != HAL_OK) {
		sprintf(buff, "Could not init GY273");
		SSD1315_Line_3(buff);
		// Handle GY-273 init failure
	}
	else
	{
		sprintf(buff, "Init GY273: OK");
		SSD1315_Line_3(buff);
	}
	Menu_Draw(&hi2c3, &guiData, NULL, NULL);
	// 1. Calibrate GY-6500 (KEEP BOARD TOTALLY STILL & FLAT!)
	sprintf(buff, "Calib GY6500: ...");
	SSD1315_Line_1(buff);
	SSD1315_UpdateScreen(&hi2c3);
	GY6500_Calibrate(&hi2c3, &imu_calib, 50);
	sprintf(buff, "Calib GY6500: Ok");
	SSD1315_Line_1(buff);
	SSD1315_UpdateScreen(&hi2c3);
	// 2. Calibrate GY-273 (ROTATE SENSOR AROUND IN AIR FOR 5 SECONDS)
	// (Optional: skip or pass NULL if hardcoding preset offsets on bench)
	sprintf(buff, "Calib GY-273: ...");
	SSD1315_Line_2(buff);
	SSD1315_UpdateScreen(&hi2c3);
	//GY273_AutoCalibrate(&hi2c3, &mag_calib, 10000);
	sprintf(buff, "Calib GY-273: Ok");
	SSD1315_Line_2(buff);
	SSD1315_UpdateScreen(&hi2c3);

	int status_pca = PCA9685_Init(&hi2c3, 50.0f);
	sprintf(buff, "Init pca9685: %s", status_pca == HAL_OK ? "Ok" : "Fail");
	SSD1315_Line_3(buff);
	SSD1315_UpdateScreen(&hi2c3);
	if (HAL_I2C_IsDeviceReady(&hi2c3, (0x40 << 1), 2, 100) != HAL_OK) {
		// I2C communication failed! Check wiring, pull-ups, or address jumpers (A0-A5).
		//Error_Handler();
	}
	HAL_Delay(status_pca == 1 ? 0 : 3000);
	HAL_TIM_Base_Start_IT(&htim2);
	SSD1315_UpdateScreen(&hi2c3);
	Menu_Draw(&hi2c3, &guiData, NULL, NULL);
	sprintf(buff, "Init CC1101: ...");
	SSD1315_Line_1(buff);
	SSD1315_UpdateScreen(&hi2c3);
	cc1101.hspi = &hspi3;
	cc1101.cs_port   = GPIOD;
	cc1101.cs_pin    = GPIO_PIN_2;
	cc1101.gdo0_port = GPIOB;
	cc1101.gdo0_pin  = GPIO_PIN_3;
	cc1101.gdo2_port = GPIOB; // Optional depending on your needs
	cc1101.gdo2_pin  = GPIO_PIN_4;       // Optional depending on your needs
	int cc1101_status = 1;
	char buff[24];
	for(uint8_t i = 1; i < 10 && cc1101_status != HAL_OK; i++)
	{
		sprintf(buff, "[CC1101 - AT:%d]", i);
		SSD1315_Title(buff);
		cc1101_status = CC1101_Init(&cc1101);
		sprintf(buff, "Init CC1101: %s", cc1101_status == HAL_OK ? "Ok" : "Fail");
		SSD1315_Line_1(buff);
		SSD1315_UpdateScreen(&hi2c3);
		HAL_Delay(100);
	}
	if(cc1101_status != HAL_OK)
	{
		sprintf(buff, "Failed to init!");
		SSD1315_Line_2(buff);
		SSD1315_UpdateScreen(&hi2c3);
		HAL_Delay(500);
	}
}
void Gyro_update_data()
{
	flag_gyro_update = 0;
	ms = ((HAL_GetTick() - imu_data.last_tick));
	imu_data = GY6500_Poll(&hi2c3, &imu_calib, &imu_data);
	mag_data = GY273_PollRaw(&hi2c3);
	heading_2d = GY273_GetHeading2D(&mag_data, &mag_calib);
	float roll = imu_data.roll * (M_PI / 180.0f);  // Roll in radians from accelerometer
	float pitch = imu_data.pitch * (M_PI / 180.0f); // Pitch in radians from accelerometer
	heading_3d = GY273_GetHeading3D(&mag_data, &mag_calib, roll, pitch);
	ori = Get_Orientation_Accel(imu_data.accel_x, imu_data.accel_y, imu_data.accel_z);
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
	/* USER CODE BEGIN 2 */
	init_all();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		if(flag_gyro_update)
		{
			Gyro_update_data();
			PCA9685_SetServoPulse(&hi2c3, 0, 1500, 50.0f);
		}
		else
		{
			Menu_Draw(&hi2c3, &guiData, &imu_data, &ori);
		}
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
	RCC_OscInitStruct.PLL.PLLM = 25;
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
	if(htim->Instance == TIM2)
	{
		flag_gyro_update = 1;
	}
}
//CC1101
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	/* Check if the interrupt came from the CC1101 GDO0 pin */
	if (GPIO_Pin == cc1101.gdo0_pin) {
		// This function will automatically trigger the DMA RX if in CC1101_STATE_RX
		CC1101_Interrupt_Handler(&cc1101);
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
	if (hspi->Instance == cc1101.hspi->Instance) {
		// Pulls CS high, triggers STX strobe
		CC1101_DMA_Complete_Callback(&cc1101);
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
	if (hspi->Instance == cc1101.hspi->Instance) {
		// Pulls CS high, extracts RSSI/LQI, puts module back into RX
		CC1101_DMA_Complete_Callback(&cc1101);

		// Notify the main loop that a packet is ready in the rx_fifo
		if (cc1101.state == CC1101_STATE_RX) {
			packet_received = true;
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
	if (hspi->Instance == cc1101.hspi->Instance) {
		// Handle DMA/SPI errors (e.g., reset CS pin, clear busy flag)
		HAL_GPIO_WritePin(cc1101.cs_port, cc1101.cs_pin, GPIO_PIN_SET);
		cc1101.dma_busy = false;

		// Re-initialize or reset state to ensure system recovers
		CC1101_SetRX(&cc1101);
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
