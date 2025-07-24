/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32h7xx_hal.h"
#include "stm32h7xx_ll_i2c.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_tim.h"
#include "stm32h7xx_ll_usart.h"
#include "stm32h7xx_ll_system.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_exti.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_cortex.h"
#include "stm32h7xx_ll_utils.h"
#include "stm32h7xx_ll_pwr.h"
#include "stm32h7xx_ll_dma.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32h7xx_ll_bdma.h"
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
#define SPI4_FLASH_CS_Pin LL_GPIO_PIN_3
#define SPI4_FLASH_CS_GPIO_Port GPIOE
#define SPI4_FRAM_CS_Pin LL_GPIO_PIN_4
#define SPI4_FRAM_CS_GPIO_Port GPIOE
#define MCU_WD_WAKE_Pin LL_GPIO_PIN_13
#define MCU_WD_WAKE_GPIO_Port GPIOC
#define SPI6_EXP_CS_Pin LL_GPIO_PIN_4
#define SPI6_EXP_CS_GPIO_Port GPIOA
#define MCU_IO_RTC_CLKOUT_Pin LL_GPIO_PIN_4
#define MCU_IO_RTC_CLKOUT_GPIO_Port GPIOC
#define MCU_IO_RTC_INT_Pin LL_GPIO_PIN_5
#define MCU_IO_RTC_INT_GPIO_Port GPIOC
#define Bootloader_DETECT_DOWN_Pin LL_GPIO_PIN_2
#define Bootloader_DETECT_DOWN_GPIO_Port GPIOB
#define STMOUT_CM4IN_SCL_Pin LL_GPIO_PIN_14
#define STMOUT_CM4IN_SCL_GPIO_Port GPIOF
#define STMOUT_CM4IN_SDA_Pin LL_GPIO_PIN_15
#define STMOUT_CM4IN_SDA_GPIO_Port GPIOF
#define CM4OUT_STMIN_D1_Pin LL_GPIO_PIN_10
#define CM4OUT_STMIN_D1_GPIO_Port GPIOE
#define CM4OUT_STMIN_D0_Pin LL_GPIO_PIN_11
#define CM4OUT_STMIN_D0_GPIO_Port GPIOE
#define OBCOUT_EXPIN_READDONE_Pin LL_GPIO_PIN_13
#define OBCOUT_EXPIN_READDONE_GPIO_Port GPIOE
#define EXPOUT_OBCIN_DATAREADY_Pin LL_GPIO_PIN_14
#define EXPOUT_OBCIN_DATAREADY_GPIO_Port GPIOE
#define MCU_SDMMC_SEL_Pin LL_GPIO_PIN_15
#define MCU_SDMMC_SEL_GPIO_Port GPIOE
#define MCU_IO_DEBUG_LED0_Pin LL_GPIO_PIN_8
#define MCU_IO_DEBUG_LED0_GPIO_Port GPIOD
#define MCU_IO_DEBUG_LED1_Pin LL_GPIO_PIN_9
#define MCU_IO_DEBUG_LED1_GPIO_Port GPIOD
#define Bootloader_DETECT_DOWND11_Pin LL_GPIO_PIN_11
#define Bootloader_DETECT_DOWND11_GPIO_Port GPIOD
#define Bootloader_DETECT_UP_Pin LL_GPIO_PIN_13
#define Bootloader_DETECT_UP_GPIO_Port GPIOD
#define Bootloader_DETECT_UPG6_Pin LL_GPIO_PIN_6
#define Bootloader_DETECT_UPG6_GPIO_Port GPIOG
#define MCU_IO_RESET_CM4_Pin LL_GPIO_PIN_7
#define MCU_IO_RESET_CM4_GPIO_Port GPIOG
#define MCU_IO_GLOBAL_EN_CM4_Pin LL_GPIO_PIN_8
#define MCU_IO_GLOBAL_EN_CM4_GPIO_Port GPIOG
#define MCU_IO_HUB_RESET_Pin LL_GPIO_PIN_3
#define MCU_IO_HUB_RESET_GPIO_Port GPIOD
#define MCU_DETECT_SD_Pin LL_GPIO_PIN_4
#define MCU_DETECT_SD_GPIO_Port GPIOD
#define MCU_WD_DONE_Pin LL_GPIO_PIN_7
#define MCU_WD_DONE_GPIO_Port GPIOD
#define EXPOUT_OBCIN_LOGTRIGGER_Pin LL_GPIO_PIN_6
#define EXPOUT_OBCIN_LOGTRIGGER_GPIO_Port GPIOB
#define EXPOUT_OBCIN_MINBUSY_Pin LL_GPIO_PIN_7
#define EXPOUT_OBCIN_MINBUSY_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
