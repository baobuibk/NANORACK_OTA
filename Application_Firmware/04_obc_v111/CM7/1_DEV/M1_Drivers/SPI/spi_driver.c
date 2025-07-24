/*
 * spi_driver.c
 *
 *  Created on: Feb 26, 2025
 *      Author: CAO HIEU
 */

#include "spi_driver.h"
#include "uart_driver_dma.h"

#define SPI_TIMEOUT_MS 10  // 10ms timeout

Std_ReturnType SPI_Driver_Transmit8(SPI_TypeDef *SPIx, uint8_t data, uint8_t *receivedData) {
    if (!LL_SPI_IsEnabled(SPIx)) return E_ERROR;

    uint32_t startTick = Utils_GetTick();
    LL_SPI_TransmitData8(SPIx, data);

    while (!LL_SPI_IsActiveFlag_TXC(SPIx)) {
        if ((Utils_GetTick() - startTick) > SPI_TIMEOUT_MS) {
            return E_TIMEOUT;
        }
    }

    startTick = Utils_GetTick();
    while (!LL_SPI_IsActiveFlag_RXP(SPIx)) {
        if ((Utils_GetTick() - startTick) > SPI_TIMEOUT_MS) {
            return E_TIMEOUT;
        }
    }

    if (receivedData) {
        *receivedData = LL_SPI_ReceiveData8(SPIx);
    }

    return E_OK;
}

Std_ReturnType SPI_Driver_Transmit16(SPI_TypeDef *SPIx, uint16_t data, uint16_t *receivedData) {
    if (!LL_SPI_IsEnabled(SPIx)) return E_ERROR;

    uint32_t startTick = Utils_GetTick();
    LL_SPI_TransmitData16(SPIx, data);

    while (!LL_SPI_IsActiveFlag_TXC(SPIx)) {
        if ((Utils_GetTick() - startTick) > SPI_TIMEOUT_MS) {
            return E_TIMEOUT;
        }
    }

    startTick = Utils_GetTick();
    while (!LL_SPI_IsActiveFlag_RXWNE(SPIx)) {
        if ((Utils_GetTick() - startTick) > SPI_TIMEOUT_MS) {
            return E_TIMEOUT;
        }
    }

    if (receivedData) {
        *receivedData = LL_SPI_ReceiveData16(SPIx);
    }

    return E_OK;
}

Std_ReturnType SPI_Driver_Write(SPI_TypeDef *SPIx, uint8_t data) {
    if (!LL_SPI_IsEnabled(SPIx)) return E_ERROR;

    uint32_t startTick = Utils_GetTick();
    LL_SPI_TransmitData8(SPIx, data);

    while (!LL_SPI_IsActiveFlag_TXC(SPIx)) {
        if ((Utils_GetTick() - startTick) > SPI_TIMEOUT_MS) {
            return E_TIMEOUT;
        }
    }

    LL_SPI_ReceiveData8(SPIx);
    return E_OK;
}

Std_ReturnType SPI_Driver_TransmitArray(SPI_TypeDef *SPIx, uint8_t *data, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        Std_ReturnType status = SPI_Driver_Write(SPIx, data[i]);
        if (status != E_OK) {
            return status;
        }
    }
    return E_OK;
}


