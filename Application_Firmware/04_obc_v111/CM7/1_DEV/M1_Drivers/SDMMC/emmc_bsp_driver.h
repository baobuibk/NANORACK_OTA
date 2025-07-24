/*
 * emmc_bsp_driver.h
 *
 *  Created on: Nov 22, 2024
 *      Author: CAO HIEU
 */

#ifndef EMMC_STM32H745_EMMC_STM32H745ZI_H_
#define EMMC_STM32H745_EMMC_STM32H745ZI_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
 /* Common Error codes */
 #define BSP_ERROR_NONE                    0
 #define BSP_ERROR_NO_INIT                -1
 #define BSP_ERROR_WRONG_PARAM            -2
 #define BSP_ERROR_BUSY                   -3
 #define BSP_ERROR_PERIPH_FAILURE         -4
 #define BSP_ERROR_COMPONENT_FAILURE      -5
 #define BSP_ERROR_UNKNOWN_FAILURE        -6
 #define BSP_ERROR_UNKNOWN_COMPONENT      -7
 #define BSP_ERROR_BUS_FAILURE            -8
 #define BSP_ERROR_CLOCK_FAILURE          -9
 #define BSP_ERROR_MSP_FAILURE            -10
 #define BSP_ERROR_FEATURE_NOT_SUPPORTED  -11
 #define BSP_ERROR_BUS_TRANSACTION_FAILURE    -100
 #define BSP_ERROR_BUS_ARBITRATION_LOSS       -101
 #define BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE    -102
 #define BSP_ERROR_BUS_PROTOCOL_FAILURE       -103

 #define BSP_ERROR_BUS_MODE_FAULT             -104
 #define BSP_ERROR_BUS_FRAME_ERROR            -105
 #define BSP_ERROR_BUS_CRC_ERROR              -106
 #define BSP_ERROR_BUS_DMA_FAILURE            -107

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H745
  * @{
  */

/** @addtogroup STM32H745_MMC
  * @{
  */

/** @defgroup STM32H745I_MMC_Exported_Types Exported Types
  * @{
  */

/**
  * @brief SD Card information structure
  */
#define BSP_MMC_CardInfo HAL_MMC_CardInfoTypeDef

#if (USE_HAL_MMC_REGISTER_CALLBACKS == 1)
typedef struct
{
  pMMC_CallbackTypeDef  pMspInitCb;
  pMMC_CallbackTypeDef  pMspDeInitCb;
}BSP_MMC_Cb_t;
#endif /* (USE_HAL_MMC_REGISTER_CALLBACKS == 1) */
/**
  * @}
  */

/** @defgroup
  * @{
  */
/**
  * @brief  MMC status structure definition
  */
#define MMC_INSTANCES_NBR             0UL

/**
  * @brief  MMC read/write timeout
  */
#ifndef MMC_WRITE_TIMEOUT
#define MMC_WRITE_TIMEOUT         500U
#endif

#ifndef MMC_READ_TIMEOUT
#define MMC_READ_TIMEOUT          500U
#endif

/**
  * @brief  MMC transfer state definition
  */
#define MMC_TRANSFER_OK               0U
#define MMC_TRANSFER_BUSY             1U

#define MMC_PRESENT                   1UL
#define MMC_NOT_PRESENT               0UL


int32_t BSP_MMC_Init(MMC_HandleTypeDef *hmmc);
int32_t BSP_MMC_DeInit(MMC_HandleTypeDef *hmmc);

int32_t BSP_MMC_ReadBlocks(MMC_HandleTypeDef *hmmc, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t BSP_MMC_WriteBlocks(MMC_HandleTypeDef *hmmc, uint32_t *pData, uint32_t BlockIdx, uint32_t NbrOfBlocks);
int32_t BSP_MMC_ReadBlocks_DMA(MMC_HandleTypeDef *hmmc, uint32_t *pData, uint32_t BlockIdx, uint32_t NbrOfBlocks);
int32_t BSP_MMC_WriteBlocks_DMA(MMC_HandleTypeDef *hmmc, uint32_t *pData, uint32_t BlockIdx, uint32_t NbrOfBlocks);
int32_t BSP_MMC_Erase(MMC_HandleTypeDef *hmmc,uint32_t StartAddr, uint32_t EndAddr);
int32_t BSP_MMC_GetCardState(MMC_HandleTypeDef *hmmc);
int32_t BSP_MMC_GetCardInfo(MMC_HandleTypeDef *hmmc, BSP_MMC_CardInfo *CardInfo);
void    BSP_MMC_IRQHandler(MMC_HandleTypeDef *hmmc);
HAL_StatusTypeDef MX_MMC_SD_Init(MMC_HandleTypeDef *hmmc);
/* These functions can be modified in case the current settings (e.g. DMA stream)
   need to be changed for specific application needs */
void    BSP_MMC_AbortCallback(MMC_HandleTypeDef *hmmc);
void    BSP_MMC_WriteCpltCallback(MMC_HandleTypeDef *hmmc);
void    BSP_MMC_ReadCpltCallback(MMC_HandleTypeDef *hmmc);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif


#endif /* EMMC_STM32H745_EMMC_STM32H745ZI_H_ */
