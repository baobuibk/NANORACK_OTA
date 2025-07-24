/*
 * spi_master.c
 *
 *  Created on: Apr 18, 2025
 *      Author: CAO HIEU
 */
#include "main.h"
#include "spi_master.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stm32h7xx_ll_bdma.h"


#define RAM_D2_200KB_SIZE (200 * 1024) // 200KB
SemaphoreHandle_t rxMaster_Semaphore = NULL;

static SPI_MasterDevice_t spi_master_instance = {
    .transfer_state = SPI_MASTER_TRANSFER_IDLE,
    .is_initialized = false,
};

SPI_MasterDevice_t* SPI_MasterDevice_GetHandle(void)
{
    return &spi_master_instance;
}

Std_ReturnType SPI_MasterDevice_Init(SPI_TypeDef *SPIx, GPIO_TypeDef *CS_Port, uint16_t CS_Pin)
{
    if (spi_master_instance.is_initialized) {
        return E_OK;
    }

    spi_master_instance.SPIx = SPIx;
    spi_master_instance.CS_Port = CS_Port;
    spi_master_instance.CS_Pin = CS_Pin;

    rxMaster_Semaphore = xSemaphoreCreateBinary();
    if (rxMaster_Semaphore == NULL) {
        return E_ERROR;
    }

    LL_BDMA_EnableIT_TC(BDMA, LL_BDMA_CHANNEL_0);
    LL_BDMA_EnableIT_TE(BDMA, LL_BDMA_CHANNEL_0);
//    LL_BDMA_SetPeriphAddress(BDMA, LL_BDMA_CHANNEL_0, (uint32_t)&SPIx->RXDR);
    LL_GPIO_SetOutputPin(CS_Port, CS_Pin);
//    LL_SPI_Enable(SPIx);

    spi_master_instance.is_initialized = true;
    spi_master_instance.transfer_state = SPI_MASTER_TRANSFER_IDLE;
    return E_OK;
}


Std_ReturnType SPI_MasterDevice_ReadDMA(uint32_t data_addr, uint32_t size)
{
    if (!spi_master_instance.is_initialized) {
        return E_ERROR;
    }

//    if (toCM4_GetState() != TOCM4_IDLE) {
//        return E_ERROR;
//    }

    if (size < 1 || size > RAM_D2_200KB_SIZE) {
        return E_ERROR;
    }

    spi_master_instance.transfer_state = SPI_MASTER_TRANSFER_BUSY;

    LL_BDMA_ConfigAddresses(BDMA, LL_BDMA_CHANNEL_1,
    						0x3800FFFF,(uint32_t) &(SPI6->TXDR),
						    LL_BDMA_GetDataTransferDirection(BDMA, LL_BDMA_CHANNEL_1));
    LL_BDMA_SetDataLength(BDMA, LL_BDMA_CHANNEL_1, size);

    LL_BDMA_ConfigAddresses(BDMA, LL_BDMA_CHANNEL_0,
                           (uint32_t) &(SPI6->RXDR), data_addr,
						   LL_BDMA_GetDataTransferDirection(BDMA, LL_BDMA_CHANNEL_0));
    LL_BDMA_SetDataLength(BDMA, LL_BDMA_CHANNEL_0, size);
    LL_BDMA_SetPeriphRequest(BDMA, LL_BDMA_CHANNEL_0, LL_DMAMUX2_REQ_SPI6_RX);

    LL_GPIO_ResetOutputPin(spi_master_instance.CS_Port, spi_master_instance.CS_Pin);

    LL_SPI_EnableDMAReq_RX(spi_master_instance.SPIx);
    LL_BDMA_EnableChannel(BDMA, LL_BDMA_CHANNEL_0);

    LL_SPI_EnableDMAReq_TX(spi_master_instance.SPIx);
    LL_BDMA_EnableChannel(BDMA, LL_BDMA_CHANNEL_1);
    LL_SPI_Enable(spi_master_instance.SPIx);
    LL_SPI_StartMasterTransfer(spi_master_instance.SPIx);

    if (xSemaphoreTake(rxMaster_Semaphore, pdMS_TO_TICKS(1000)) != pdTRUE) {
        LL_BDMA_DisableChannel(BDMA, LL_BDMA_CHANNEL_1);
        LL_BDMA_DisableChannel(BDMA, LL_BDMA_CHANNEL_0);
        LL_SPI_DisableDMAReq_RX(spi_master_instance.SPIx);
        LL_SPI_DisableDMAReq_TX(spi_master_instance.SPIx);
        LL_SPI_Disable(spi_master_instance.SPIx);
        LL_GPIO_SetOutputPin(spi_master_instance.CS_Port, spi_master_instance.CS_Pin);
        toCM4_SetState(TOCM4_ERROR);
        return E_ERROR;
    }

    LL_BDMA_DisableChannel(BDMA, LL_BDMA_CHANNEL_1);
    LL_BDMA_DisableChannel(BDMA, LL_BDMA_CHANNEL_0);
    LL_SPI_DisableDMAReq_RX(spi_master_instance.SPIx);
    LL_SPI_DisableDMAReq_TX(spi_master_instance.SPIx);
    LL_SPI_Disable(spi_master_instance.SPIx);
    LL_GPIO_SetOutputPin(spi_master_instance.CS_Port, spi_master_instance.CS_Pin);

    if (spi_master_instance.transfer_state == SPI_MASTER_TRANSFER_COMPLETE) {
        toCM4_SetState(TOCM4_READYSEND);
        return E_OK;
    } else {
        toCM4_SetState(TOCM4_ERROR);
        return E_ERROR;
    }
}

Std_ReturnType SPI_MasterDevice_Disable(void)
{
    if (!spi_master_instance.is_initialized) {
        return E_ERROR;
    }

    LL_BDMA_DisableChannel(BDMA, LL_BDMA_CHANNEL_0);
    LL_SPI_DisableDMAReq_RX(spi_master_instance.SPIx);
    LL_SPI_Disable(spi_master_instance.SPIx);
    LL_GPIO_SetOutputPin(spi_master_instance.CS_Port, spi_master_instance.CS_Pin);

    LL_BDMA_ClearFlag_TC0(BDMA);
    LL_BDMA_ClearFlag_TE0(BDMA);

    spi_master_instance.transfer_state = SPI_MASTER_TRANSFER_IDLE;
    toCM4_SetState(TOCM4_IDLE);
    return E_OK;
}

SPI_MasterTransferState_t SPI_MasterDevice_GetTransferState(void)
{
    return spi_master_instance.transfer_state;
}

void SPI_MasterDevice_SetTransferState(SPI_MasterTransferState_t state)
{
    spi_master_instance.transfer_state = state;
}

toCM4_State_t SPI_MasterDevice_GetCM4State(void)
{
    return toCM4_GetState();
}

void SPIMaster_IRQHandler(void)
{
    if (LL_BDMA_IsActiveFlag_TC0(BDMA)) {
        LL_BDMA_ClearFlag_TC0(BDMA);
        SPI_MasterDevice_SetTransferState(SPI_MASTER_TRANSFER_COMPLETE);
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(rxMaster_Semaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    if (LL_BDMA_IsActiveFlag_TE0(BDMA)) {
        LL_BDMA_ClearFlag_TE0(BDMA);
        SPI_MasterDevice_SetTransferState(SPI_MASTER_TRANSFER_ERROR);
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(rxMaster_Semaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


