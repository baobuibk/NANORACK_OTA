/*
 * spi_slave.c
 *
 *  Created on: Apr 17, 2025
 *      Author: CAO HIEU
 */

#include "spi_slave.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_dma.h"

#define RAM_D2_200KB_SIZE (200 * 1024) // 200KB

#define SPI_SLAVE_INSTANCE SPI5
#define SPI_DMA_INSTANCE DMA1
#define SPI_DMA_TX_STREAM LL_DMA_STREAM_7

static SPI_SlaveDevice_t spi_device_instance = {
    .transfer_state = SPI_TRANSFER_WAIT,
    .data_context = {0},
    .is_initialized = false
};

static uint16_t UpdateCRC16_XMODEM(uint16_t crc, uint8_t byte) {
    const uint16_t polynomial = 0x1021; // CRC16 XMODEM
    crc ^= (uint16_t)byte << 8;
    for (uint8_t bit = 0; bit < 8; bit++) {
        if (crc & 0x8000) {
            crc = (crc << 1) ^ polynomial;
        } else {
            crc <<= 1;
        }
    }
    return crc;
}

SPI_SlaveDevice_t* SPI_SlaveDevice_GetHandle(void)
{
    return &spi_device_instance;
}

Std_ReturnType SPI_SlaveDevice_Init(void)
{
    if (spi_device_instance.is_initialized) {
        return E_OK;
    }

    LL_DMA_SetMemorySize(SPI_DMA_INSTANCE, SPI_DMA_TX_STREAM, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_SetPeriphAddress(SPI_DMA_INSTANCE, SPI_DMA_TX_STREAM, (uint32_t)&SPI_SLAVE_INSTANCE->TXDR);

    LL_DMA_EnableIT_TC(SPI_DMA_INSTANCE, SPI_DMA_TX_STREAM);
    LL_DMA_EnableIT_TE(SPI_DMA_INSTANCE, SPI_DMA_TX_STREAM);

    toCM4_Init();
    spi_device_instance.data_context.is_valid = false;
    spi_device_instance.is_initialized = true;
    spi_device_instance.transfer_state = SPI_TRANSFER_WAIT;
    return E_OK;
}

Std_ReturnType SPI_SlaveDevice_CollectData(uint8_t type, uint32_t sample, uint32_t data_addr)
{
    if (!spi_device_instance.is_initialized) {
        return E_ERROR;
    }

    if (toCM4_GetState() != TOCM4_IDLE) {
        return E_ERROR;
    }

    if (type > 3) {
        return E_ERROR;
    }

    if (sample < 1 || sample > 100000 || sample * 2 > RAM_D2_200KB_SIZE) {
        return E_ERROR;
    }

    toCM4_SetState(TOCM4_BUSY);
    spi_device_instance.data_context.is_valid = false;

    if (type == 0) {
        uint16_t crc = 0x0000;
        uint16_t *data = (uint16_t *)data_addr;

        for (uint32_t i = 0; i < sample; i++) {
            uint16_t value = (uint16_t)(i % 1001);
            data[i] = value;
            crc = UpdateCRC16_XMODEM(crc, (uint8_t)(value & 0xFF));
            crc = UpdateCRC16_XMODEM(crc, (uint8_t)((value >> 8) & 0xFF));
        }

        spi_device_instance.data_context.type = type;
        spi_device_instance.data_context.sample = sample;
        spi_device_instance.data_context.data_size = sample * 2;
        spi_device_instance.data_context.crc = crc;
        spi_device_instance.data_context.is_valid = true;

        if (SPI_SlaveDevice_ResetDMA(data_addr, spi_device_instance.data_context.data_size) != E_OK) {
            toCM4_SetState(TOCM4_ERROR);
            return E_ERROR;
        }

        toCM4_SetState(TOCM4_READYSEND);
    } else {
        toCM4_SetState(TOCM4_IDLE);
        return E_BUSY;
    }

    return E_OK;
}

Std_ReturnType SPI_SlaveDevice_GetDataInfo(DataProcessContext_t *context)
{
    if (!spi_device_instance.is_initialized || !spi_device_instance.data_context.is_valid) {
        return E_ERROR;
    }

    *context = spi_device_instance.data_context;
    return E_OK;
}

Std_ReturnType SPI_SlaveDevice_ResetDMA(uint32_t data_addr, uint32_t data_size)
{
    if (!spi_device_instance.is_initialized) {
        return E_ERROR;
    }

    if (toCM4_GetState() != TOCM4_BUSY && toCM4_GetState() != TOCM4_READYSEND) {
        return E_ERROR;
    }

    LL_DMA_DisableStream(SPI_DMA_INSTANCE, SPI_DMA_TX_STREAM);
    LL_SPI_DisableDMAReq_TX(SPI_SLAVE_INSTANCE);
    LL_SPI_Disable(SPI_SLAVE_INSTANCE);

    LL_DMA_ClearFlag_TC7(SPI_DMA_INSTANCE);
    LL_DMA_ClearFlag_TE7(SPI_DMA_INSTANCE);

    LL_DMA_ConfigAddresses(SPI_DMA_INSTANCE, SPI_DMA_TX_STREAM,
                           data_addr, (uint32_t)&SPI_SLAVE_INSTANCE->TXDR,
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetDataLength(SPI_DMA_INSTANCE, SPI_DMA_TX_STREAM, data_size);

    LL_SPI_Enable(SPI_SLAVE_INSTANCE);
    LL_SPI_EnableDMAReq_TX(SPI_SLAVE_INSTANCE);
    LL_DMA_EnableStream(SPI_DMA_INSTANCE, SPI_DMA_TX_STREAM);

    spi_device_instance.transfer_state = SPI_TRANSFER_WAIT;
    return E_OK;
}

Std_ReturnType SPI_SlaveDevice_Disable(void)
{
    if (!spi_device_instance.is_initialized) {
        return E_ERROR;
    }

    LL_DMA_DisableStream(SPI_DMA_INSTANCE, SPI_DMA_TX_STREAM);
    LL_SPI_DisableDMAReq_TX(SPI_SLAVE_INSTANCE);
//    LL_SPI_Disable(SPI_SLAVE_INSTANCE);

    LL_DMA_ClearFlag_TC7(SPI_DMA_INSTANCE);
    LL_DMA_ClearFlag_TE7(SPI_DMA_INSTANCE);

    spi_device_instance.transfer_state = SPI_TRANSFER_WAIT;
    spi_device_instance.data_context.is_valid = false;
    toCM4_SetState(TOCM4_IDLE);
    return E_OK;
}

SPI_TransferState_t SPI_SlaveDevice_GetTransferState(void)
{
    return spi_device_instance.transfer_state;
}

void SPI_SlaveDevice_SetTransferState(SPI_TransferState_t state)
{
    spi_device_instance.transfer_state = state;
}

toCM4_State_t SPI_SlaveDevice_GetCM4State(void)
{
    return toCM4_GetState();
}
