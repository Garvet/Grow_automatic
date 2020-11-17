/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LoRa_NSS_Pin GPIO_PIN_4
#define LoRa_NSS_GPIO_Port GPIOA
#define LoRa_DIO1_Pin GPIO_PIN_2
#define LoRa_DIO1_GPIO_Port GPIOB
#define LoRa_DIO0_Pin GPIO_PIN_10
#define LoRa_DIO0_GPIO_Port GPIOB
#define LoRa_RESET_Pin GPIO_PIN_11
#define LoRa_RESET_GPIO_Port GPIOB
#define LED1_PIN_Pin GPIO_PIN_8
#define LED1_PIN_GPIO_Port GPIOA
#define LED2_PIN_Pin GPIO_PIN_9
#define LED2_PIN_GPIO_Port GPIOA
#define LED3_PIN_Pin GPIO_PIN_10
#define LED3_PIN_GPIO_Port GPIOA
#define RELAY3_PIN_Pin GPIO_PIN_3
#define RELAY3_PIN_GPIO_Port GPIOB
#define RELAY2_PIN_Pin GPIO_PIN_4
#define RELAY2_PIN_GPIO_Port GPIOB
#define RELAY1_PIN_Pin GPIO_PIN_5
#define RELAY1_PIN_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
