#ifndef TARGET_BOARD_H_
#define TARGET_BOARD_H_

#include "env.h"

#define BOARD_NAME					"OBC"
#define VERSION 					"1.1.1"

#define MCU							"STM32H745ZIT3"
#define CORE						"M7"

#define USE_FREERTOS

#define UART_DEBUG					USART2//UART7

#define UART_EXP					UART7//USART2

#define UART_USB					USART1

#define I2C_RTC						I2C2

#define SPI_MEM						SPI4

#define LED0						MCU_IO_DEBUG_LED0_Pin
#define LED0_Port					MCU_IO_DEBUG_LED0_GPIO_Port

#define LED1						MCU_IO_DEBUG_LED1_Pin
#define LED1_Port					MCU_IO_DEBUG_LED1_GPIO_Port

#define CM4_RST_Pin					MCU_IO_RESET_CM4_Pin
#define CM4_RST_Port				MCU_IO_RESET_CM4_GPIO_Port

#define CM4_ENA_Pin					MCU_IO_GLOBAL_EN_CM4_Pin
#define CM4_ENA_Port				MCU_IO_GLOBAL_EN_CM4_GPIO_Port

#define WD_Done_Pin					MCU_WD_DONE_Pin
#define WD_Done_Port				MCU_WD_DONE_GPIO_Port

#define LED3						MCU_IO_DEBUG_LED3_Pin
#define LED3_Port					MCU_IO_DEBUG_LED3_GPIO_Port

#define LED2						MCU_IO_DEBUG_LED2_Pin
#define LED2_Port					MCU_IO_DEBUG_LED2_GPIO_Port

#define SD_InOut					MCU_SDMMC_SEL_Pin
#define SD_InOut_Port				MCU_SDMMC_SEL_GPIO_Port

#define SD_Detect					MCU_DETECT_SD_Pin
#define SD_Detect_Port				MCU_DETECT_SD_GPIO_Port

#define FRAM_CS						SPI4_FRAM_CS_Pin
#define FRAM_CS_Port				SPI4_FRAM_CS_GPIO_Port

//#define STATE_toCM4_BUSY			I2C4SCL_BUSY_STATE_Pin
//#define STATE_toCM4_BUSY_Port		I2C4SCL_BUSY_STATE_GPIO_Port
//
//#define STATE_toCM4_READYSEND		I2C4SDA_READYSEND_STATE_Pin
//#define STATE_toCM4_READYSEND_Port  I2C4SDA_READYSEND_STATE_GPIO_Port

#endif /* TARGET_BOARD_H_ */
