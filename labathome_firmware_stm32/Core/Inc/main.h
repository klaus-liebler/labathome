/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32g4xx_hal.h"

#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_crs.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_exti.h"
#include "stm32g4xx_ll_cortex.h"
#include "stm32g4xx_ll_utils.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_ucpd.h"
#include "stm32g4xx_ll_gpio.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ENC_A_Pin LL_GPIO_PIN_0
#define ENC_A_GPIO_Port GPIOA
#define ENC_B_Pin LL_GPIO_PIN_1
#define ENC_B_GPIO_Port GPIOA
#define Servo1_Pin LL_GPIO_PIN_2
#define Servo1_GPIO_Port GPIOA
#define Servo2_Pin LL_GPIO_PIN_3
#define Servo2_GPIO_Port GPIOA
#define Servo3_Pin LL_GPIO_PIN_6
#define Servo3_GPIO_Port GPIOA
#define Fan_Pin LL_GPIO_PIN_7
#define Fan_GPIO_Port GPIOA
#define Heater_Pin LL_GPIO_PIN_4
#define Heater_GPIO_Port GPIOC
#define Hall2_Pin LL_GPIO_PIN_0
#define Hall2_GPIO_Port GPIOB
#define Hall3_Pin LL_GPIO_PIN_1
#define Hall3_GPIO_Port GPIOB
#define BRIGHTNESS_Pin LL_GPIO_PIN_2
#define BRIGHTNESS_GPIO_Port GPIOB
#define RELAY_Pin LL_GPIO_PIN_11
#define RELAY_GPIO_Port GPIOB
#define BL_RESET_Pin LL_GPIO_PIN_12
#define BL_RESET_GPIO_Port GPIOB
#define BL_FAULT_Pin LL_GPIO_PIN_13
#define BL_FAULT_GPIO_Port GPIOB
#define ADC1_Pin LL_GPIO_PIN_14
#define ADC1_GPIO_Port GPIOB
#define ADC2_Pin LL_GPIO_PIN_15
#define ADC2_GPIO_Port GPIOB
#define BL_ENABLE_OR_LED_Pin LL_GPIO_PIN_6
#define BL_ENABLE_OR_LED_GPIO_Port GPIOC
#define BL_DRV1_Pin LL_GPIO_PIN_8
#define BL_DRV1_GPIO_Port GPIOA
#define BL_DRV2_Pin LL_GPIO_PIN_9
#define BL_DRV2_GPIO_Port GPIOA
#define BL_DRV3_Pin LL_GPIO_PIN_10
#define BL_DRV3_GPIO_Port GPIOA
#define LED_INFO_Pin LL_GPIO_PIN_3
#define LED_INFO_GPIO_Port GPIOB
#define BL_HALL3_Pin LL_GPIO_PIN_5
#define BL_HALL3_GPIO_Port GPIOB
#define BTN_RED_Pin LL_GPIO_PIN_8
#define BTN_RED_GPIO_Port GPIOB
#define LED_POWER_Pin LL_GPIO_PIN_9
#define LED_POWER_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
