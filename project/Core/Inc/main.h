/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#define KEY_0_Pin GPIO_PIN_2
#define KEY_0_GPIO_Port GPIOE
#define KEY_1_Pin GPIO_PIN_3
#define KEY_1_GPIO_Port GPIOE
#define KEY2_Pin GPIO_PIN_4
#define KEY2_GPIO_Port GPIOE
#define LED_Pin GPIO_PIN_9
#define LED_GPIO_Port GPIOF
#define KEY_WK_Pin GPIO_PIN_0
#define KEY_WK_GPIO_Port GPIOA
#define PORT_DAC_Pin GPIO_PIN_4
#define PORT_DAC_GPIO_Port GPIOA
#define PORT_ADC_Pin GPIO_PIN_6
#define PORT_ADC_GPIO_Port GPIOA
#define ETH_ENABLE_Pin GPIO_PIN_3
#define ETH_ENABLE_GPIO_Port GPIOD
#define PORT_GAIN_A_Pin GPIO_PIN_6
#define PORT_GAIN_A_GPIO_Port GPIOD
#define PORT_GAIN_B_Pin GPIO_PIN_7
#define PORT_GAIN_B_GPIO_Port GPIOD
#define PORT_TRIG_Pin GPIO_PIN_15
#define PORT_TRIG_GPIO_Port GPIOG
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
