/**
  ******************************************************************************
  * @file    FatFs/FatFs_Shared_Device/Common/Src/mmc_diskio.c
  * @author  MCD Application Team
  * @brief   MMC Disk I/O FatFs driver
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include <stdio.h>
#include "ff_gen_drv.h"
#include "mmc_diskio.h"

#define USE_FREERTOS

#ifdef USE_FREERTOS
//#include "FreeRTOS.h"
//#include "semphr.h"
//#include "task.h"
#endif

/* External MMC handle declarations */
#if defined(DUAL_MMC) || defined(ONLY_MMC1)
extern MMC_HandleTypeDef hmmc1;
#endif
#if defined(DUAL_MMC) || defined(ONLY_MMC2)
extern MMC_HandleTypeDef hmmc2;
#endif

/* Private defines */
#define MMC_TIMEOUT 30000
#define MMC_DEFAULT_BLOCK_SIZE 512
#define ENABLE_DMA_CACHE_MAINTENANCE 0
#define EMMC_HSEM_ID (1U)

#define LOCK_HSEM(__sem__)    do { while(HAL_HSEM_FastTake(__sem__) != HAL_OK) {} } while(0)
#define UNLOCK_HSEM(__sem__)  HAL_HSEM_Release(__sem__, 0)

/* Private variables */
static volatile DSTATUS Stat = STA_NOINIT;

#ifdef USE_FREERTOS
#if defined(DUAL_MMC) || defined(ONLY_MMC1)
static SemaphoreHandle_t MMC1Semaphore = NULL;  /* Semaphore for MMC1 */
#endif
#if defined(DUAL_MMC) || defined(ONLY_MMC2)
static SemaphoreHandle_t MMC2Semaphore = NULL;  /* Semaphore for MMC2 */
#endif
#else
static volatile UINT WriteStatus = 0, ReadStatus = 0;
#endif

/* Generic MMC driver implementation */
static DSTATUS MMC_CheckStatus(MMC_HandleTypeDef *hmmc) {
    Stat = STA_NOINIT;
    if (BSP_MMC_GetCardState(hmmc) == BSP_ERROR_NONE) {
        Stat &= ~STA_NOINIT;
    }
    return Stat;
}

static DSTATUS MMC_Initialize(MMC_HandleTypeDef *hmmc, BYTE lun) {
#ifdef USE_FREERTOS
    /* Ensure FreeRTOS kernel is running before using RTOS APIs */
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
#ifndef DISABLE_MMC_INIT
        if (BSP_MMC_Init(hmmc) == BSP_ERROR_NONE) {
            Stat = MMC_CheckStatus(hmmc);
        }
#else
        Stat = MMC_CheckStatus(hmmc);
#endif
        /* Create semaphore for DMA synchronization if MMC is initialized */
        if (Stat != STA_NOINIT) {
#if defined(DUAL_MMC) || defined(ONLY_MMC1)
            if (hmmc == &hmmc1 && MMC1Semaphore == NULL) {
                MMC1Semaphore = xSemaphoreCreateBinary();
                if (MMC1Semaphore == NULL) {
                    Stat = STA_NOINIT;  /* Failed to create semaphore */
                }
            }
#endif
#if defined(DUAL_MMC) || defined(ONLY_MMC2)
            if (hmmc == &hmmc2 && MMC2Semaphore == NULL) {
                MMC2Semaphore = xSemaphoreCreateBinary();
                if (MMC2Semaphore == NULL) {
                    Stat = STA_NOINIT;  /* Failed to create semaphore */
                }
            }
#endif
        }
    }
#else
#ifndef DISABLE_MMC_INIT
    if (BSP_MMC_Init(hmmc) == BSP_ERROR_NONE) {
        return MMC_CheckStatus(hmmc);
    }
#else
    return MMC_CheckStatus(hmmc);
#endif
#endif
    return Stat;
}

static DRESULT MMC_Read(MMC_HandleTypeDef *hmmc, BYTE *buff, DWORD sector, UINT count) {
    DRESULT res = RES_ERROR;

#ifdef USE_FREERTOS
#if defined(DUAL_MMC) || defined(ONLY_MMC1)
    SemaphoreHandle_t semaphore = (hmmc == &hmmc1) ? MMC1Semaphore :
#endif
#if defined(DUAL_MMC) || defined(ONLY_MMC2)
                                  (hmmc == &hmmc2) ? MMC2Semaphore : NULL;
#else
                                  NULL;
#endif

#if (ENABLE_DMA_CACHE_MAINTENANCE == 1)
    uint32_t alignedAddr = (uint32_t)buff & ~0x1F;
    SCB_InvalidateDCache_by_Addr((uint32_t*)alignedAddr, count * MMC_DEFAULT_BLOCK_SIZE + ((uint32_t)buff - alignedAddr));
#endif
    if (BSP_MMC_ReadBlocks_DMA(hmmc, (uint32_t*)buff, sector, count) == BSP_ERROR_NONE) {
        /* Wait for DMA completion via semaphore */
        if (xSemaphoreTake(semaphore, pdMS_TO_TICKS(MMC_TIMEOUT)) == pdTRUE) {
            /* Check MMC state to ensure transfer is complete */
            TickType_t timeout = xTaskGetTickCount() + pdMS_TO_TICKS(MMC_TIMEOUT);
            while (xTaskGetTickCount() < timeout) {
                if (BSP_MMC_GetCardState(hmmc) == MMC_TRANSFER_OK) {
                    res = RES_OK;
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(1));  /* Delay 1ms to avoid busy-wait */
            }
        }
    }
#else
    ReadStatus = 0;
    uint32_t timeout;
    if (BSP_MMC_ReadBlocks_DMA(hmmc, (uint32_t*)buff, sector, count) == BSP_ERROR_NONE) {
        timeout = HAL_GetTick();
        while ((ReadStatus == 0) && ((HAL_GetTick() - timeout) < MMC_TIMEOUT)) {}
        if (ReadStatus) {
            ReadStatus = 0;
            timeout = HAL_GetTick();
            while ((HAL_GetTick() - timeout) < MMC_TIMEOUT) {
                if (BSP_MMC_GetCardState(hmmc) == MMC_TRANSFER_OK) {
                    res = RES_OK;
                    break;
                }
            }
        }
    }
#endif
    return res;
}

#if _USE_WRITE == 1
static DRESULT MMC_Write(MMC_HandleTypeDef *hmmc, const BYTE *buff, DWORD sector, UINT count) {
    DRESULT res = RES_ERROR;

#ifdef USE_FREERTOS
#if defined(DUAL_MMC) || defined(ONLY_MMC1)
    SemaphoreHandle_t semaphore = (hmmc == &hmmc1) ? MMC1Semaphore :
#endif
#if defined(DUAL_MMC) || defined(ONLY_MMC2)
                                  (hmmc == &hmmc2) ? MMC2Semaphore : NULL;
#else
                                  NULL;
#endif

#if (ENABLE_DMA_CACHE_MAINTENANCE == 1)
    uint32_t alignedAddr = (uint32_t)buff & ~0x1F;
    SCB_CleanDCache_by_Addr((uint32_t*)alignedAddr, count * MMC_DEFAULT_BLOCK_SIZE + ((uint32_t)buff - alignedAddr));
#endif
    if (BSP_MMC_WriteBlocks_DMA(hmmc, (uint32_t*)buff, sector, count) == BSP_ERROR_NONE) {
        /* Wait for DMA completion via semaphore */
        if (xSemaphoreTake(semaphore, pdMS_TO_TICKS(MMC_TIMEOUT)) == pdTRUE) {
            /* Check MMC state to ensure transfer is complete */
            TickType_t timeout = xTaskGetTickCount() + pdMS_TO_TICKS(MMC_TIMEOUT);
            while (xTaskGetTickCount() < timeout) {
                if (BSP_MMC_GetCardState(hmmc) == MMC_TRANSFER_OK) {
                    res = RES_OK;
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(1));  /* Delay 1ms to avoid busy-wait */
            }
        }
    }
#else
    WriteStatus = 0;
    uint32_t timeout;
    if (BSP_MMC_WriteBlocks_DMA(hmmc, (uint32_t*)buff, sector, count) == BSP_ERROR_NONE) {
        timeout = HAL_GetTick();
        while ((WriteStatus == 0) && ((HAL_GetTick() - timeout) < MMC_TIMEOUT)) {}
        if (WriteStatus) {
            WriteStatus = 0;
            timeout = HAL_GetTick();
            while ((HAL_GetTick() - timeout) < MMC_TIMEOUT) {
                if (BSP_MMC_GetCardState(hmmc) == MMC_TRANSFER_OK) {
                    res = RES_OK;
                    break;
                }
            }
        }
    }
#endif
    return res;
}
#endif

#if _USE_IOCTL == 1
static DRESULT MMC_Ioctl(MMC_HandleTypeDef *hmmc, BYTE cmd, void *buff) {
    DRESULT res = RES_ERROR;
    BSP_MMC_CardInfo CardInfo;

    if (Stat & STA_NOINIT) return RES_NOTRDY;

    switch (cmd) {
        case CTRL_SYNC:
            res = RES_OK;
            break;
        case GET_SECTOR_COUNT:
            BSP_MMC_GetCardInfo(hmmc, &CardInfo);
            *(DWORD*)buff = CardInfo.LogBlockNbr;
            res = RES_OK;
            break;
        case GET_SECTOR_SIZE:
            BSP_MMC_GetCardInfo(hmmc, &CardInfo);
            *(WORD*)buff = CardInfo.LogBlockSize;
            res = RES_OK;
            break;
        case GET_BLOCK_SIZE:
            BSP_MMC_GetCardInfo(hmmc, &CardInfo);
            *(DWORD*)buff = CardInfo.LogBlockSize / MMC_DEFAULT_BLOCK_SIZE;
            res = RES_OK;
            break;
        default:
            res = RES_PARERR;
    }
    return res;
}
#endif

/* Callback functions */
void BSP_MMC_WriteCpltCallback(MMC_HandleTypeDef *hmmc) {
#ifdef USE_FREERTOS
#if defined(DUAL_MMC) || defined(ONLY_MMC1)
    SemaphoreHandle_t semaphore = (hmmc == &hmmc1) ? MMC1Semaphore :
#endif
#if defined(DUAL_MMC) || defined(ONLY_MMC2)
                                  (hmmc == &hmmc2) ? MMC2Semaphore : NULL;
#else
                                  NULL;
#endif
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(semaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#else
    WriteStatus = 1;
#endif
}

void BSP_MMC_ReadCpltCallback(MMC_HandleTypeDef *hmmc) {
#ifdef USE_FREERTOS
#if defined(DUAL_MMC) || defined(ONLY_MMC1)
    SemaphoreHandle_t semaphore = (hmmc == &hmmc1) ? MMC1Semaphore :
#endif
#if defined(DUAL_MMC) || defined(ONLY_MMC2)
                                  (hmmc == &hmmc2) ? MMC2Semaphore : NULL;
#else
                                  NULL;
#endif
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(semaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#else
    ReadStatus = 1;
#endif
}

/* MMC1 Driver */
#if defined(DUAL_MMC) || defined(ONLY_MMC1)
static DSTATUS MMC1_initialize(BYTE lun) { return MMC_Initialize(&hmmc1, lun); }
static DSTATUS MMC1_status(BYTE lun) { return MMC_CheckStatus(&hmmc1); }
static DRESULT MMC1_read(BYTE lun, BYTE *buff, DWORD sector, UINT count) { return MMC_Read(&hmmc1, buff, sector, count); }
#if _USE_WRITE == 1
static DRESULT MMC1_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count) { return MMC_Write(&hmmc1, buff, sector, count); }
#endif
#if _USE_IOCTL == 1
static DRESULT MMC1_ioctl(BYTE lun, BYTE cmd, void *buff) { return MMC_Ioctl(&hmmc1, cmd, buff); }
#endif

static const Diskio_drvTypeDef MMC1_Driver = {
    MMC1_initialize,
    MMC1_status,
    MMC1_read,
#if _USE_WRITE == 1
    MMC1_write,
#endif
#if _USE_IOCTL == 1
    MMC1_ioctl,
#endif
};

const Diskio_drvTypeDef* MMC1_GetDriver(void) {
    return &MMC1_Driver;
}
#endif

/* MMC2 Driver */
#if defined(DUAL_MMC) || defined(ONLY_MMC2)
static DSTATUS MMC2_initialize(BYTE lun) { return MMC_Initialize(&hmmc2, lun); }
static DSTATUS MMC2_status(BYTE lun) { return MMC_CheckStatus(&hmmc2); }
static DRESULT MMC2_read(BYTE lun, BYTE *buff, DWORD sector, UINT count) { return MMC_Read(&hmmc2, buff, sector, count); }
#if _USE_WRITE == 1
static DRESULT MMC2_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count) { return MMC_Write(&hmmc2, buff, sector, count); }
#endif
#if _USE_IOCTL == 1
static DRESULT MMC2_ioctl(BYTE lun, BYTE cmd, void *buff) { return MMC_Ioctl(&hmmc2, cmd, buff); }
#endif

static const Diskio_drvTypeDef MMC2_Driver = {
    MMC2_initialize,
    MMC2_status,
    MMC2_read,
#if _USE_WRITE == 1
    MMC2_write,
#endif
#if _USE_IOCTL == 1
    MMC2_ioctl,
#endif
};

const Diskio_drvTypeDef* MMC2_GetDriver(void) {
    return &MMC2_Driver;
}
#endif
