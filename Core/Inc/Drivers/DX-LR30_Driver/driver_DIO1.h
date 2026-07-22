#ifndef __DRIVER_DIO1_H
#define __DRIVER_DIO1_H

#include "main.h"

/*******************************************************************************
 * Hardware Pin Mapping
 ******************************************************************************/
#define LORA_DIO1_PIN               GPIO_PIN_3
#define LORA_DIO1_PORT              GPIOB
#define LORA_DIO1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOG_CLK_ENABLE()

/*******************************************************************************
 * Macro Definitions
 ******************************************************************************/
/* Read current hardware pin state of DIO1 line */
#define READ_DIO1                   HAL_GPIO_ReadPin(LORA_DIO1_PORT, LORA_DIO1_PIN)

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

#endif /* __DRIVER_DIO1_H */
