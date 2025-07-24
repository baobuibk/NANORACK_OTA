/*
 * spi_fram.c
 *
 *  Created on: Feb 26, 2025
 *      Author: CAO HIEU
 */
#include "fram_spi.h"
#include "spi_driver.h"

#define FRAM_WREN   0x06   // Write enable
#define FRAM_WRDI   0x04   // Write disable
#define FRAM_RDSR   0x05   // Read status
#define FRAM_WRSR   0x01   // Write status
#define FRAM_READ   0x03   // Read memory
#define FRAM_WRITE  0x02   // Write memory
#define FRAM_RDID   0x9F   // Read device ID
#define FRAM_FSTRD  0x0B   // Fast read memory
#define FRAM_SLEEP  0xB9   // Sleep mode

static FRAM_SPI_HandleTypeDef hfram_instance;

FRAM_SPI_HandleTypeDef* FRAM_SPI_GetHandle(void)
{
    return &hfram_instance;
}

void FRAM_SPI_Driver_Init(SPI_TypeDef *SPIx, GPIO_TypeDef *CS_Port, uint16_t CS_Pin)
{
    hfram_instance.SPIx = SPIx;
    hfram_instance.CS_Port = CS_Port;
    hfram_instance.CS_Pin = CS_Pin;

    LL_GPIO_SetOutputPin(CS_Port, CS_Pin);
}

Std_ReturnType FRAM_SPI_WriteEnable(FRAM_SPI_HandleTypeDef *hfram)
{
    Std_ReturnType status;

    LL_GPIO_ResetOutputPin(hfram->CS_Port, hfram->CS_Pin);
    status = SPI_Driver_Write(hfram->SPIx, FRAM_WREN);
    LL_GPIO_SetOutputPin(hfram->CS_Port, hfram->CS_Pin);

    return status;
}

Std_ReturnType FRAM_SPI_WriteDisable(FRAM_SPI_HandleTypeDef *hfram)
{
    Std_ReturnType status;

    LL_GPIO_ResetOutputPin(hfram->CS_Port, hfram->CS_Pin);
    status = SPI_Driver_Write(hfram->SPIx, FRAM_WRDI);
    LL_GPIO_SetOutputPin(hfram->CS_Port, hfram->CS_Pin);

    return status;
}

Std_ReturnType FRAM_SPI_WriteMem(FRAM_SPI_HandleTypeDef *hfram, uint32_t addr, uint8_t *pData, uint16_t len)
{
    Std_ReturnType status;
    uint8_t addr_bytes[3] = {
        (uint8_t)((addr >> 16) & 0xFF),
        (uint8_t)((addr >> 8) & 0xFF),
        (uint8_t)(addr & 0xFF)
    };

    status = FRAM_SPI_WriteEnable(hfram);
    if(status != E_OK) return status;

    LL_GPIO_ResetOutputPin(hfram->CS_Port, hfram->CS_Pin);

    status = SPI_Driver_Write(hfram->SPIx, FRAM_WRITE);
    if(status != E_OK) {
        LL_GPIO_SetOutputPin(hfram->CS_Port, hfram->CS_Pin);
        return status;
    }

    for (int i = 0; i < 3; i++)
    {
        status = SPI_Driver_Write(hfram->SPIx, addr_bytes[i]);
        if(status != E_OK) {
            LL_GPIO_SetOutputPin(hfram->CS_Port, hfram->CS_Pin);
            return status;
        }
    }

    for (int i = 0; i < len; i++)
    {
        status = SPI_Driver_Write(hfram->SPIx, pData[i]);
        if(status != E_OK) {
            LL_GPIO_SetOutputPin(hfram->CS_Port, hfram->CS_Pin);
            return status;
        }
    }

    LL_GPIO_SetOutputPin(hfram->CS_Port, hfram->CS_Pin);

    status = FRAM_SPI_WriteDisable(hfram);

    return status;
}

Std_ReturnType FRAM_SPI_ReadMem(FRAM_SPI_HandleTypeDef *hfram, uint32_t addr, uint8_t *pData, uint16_t len)
{
    Std_ReturnType status;
    uint8_t addr_bytes[3] = {
        (uint8_t)((addr >> 16) & 0xFF),
        (uint8_t)((addr >> 8) & 0xFF),
        (uint8_t)(addr & 0xFF)
    };

    LL_GPIO_ResetOutputPin(hfram->CS_Port, hfram->CS_Pin);

    status = SPI_Driver_Write(hfram->SPIx, FRAM_READ);
    if(status != E_OK) {
        LL_GPIO_SetOutputPin(hfram->CS_Port, hfram->CS_Pin);
        return status;
    }

    for (int i = 0; i < 3; i++)
    {
        status = SPI_Driver_Write(hfram->SPIx, addr_bytes[i]);
        if(status != E_OK) {
            LL_GPIO_SetOutputPin(hfram->CS_Port, hfram->CS_Pin);
            return status;
        }
    }

    for (int i = 0; i < len; i++)
    {
        status = SPI_Driver_Transmit8(hfram->SPIx, 0x00, &pData[i]);
        if(status != E_OK) {
            LL_GPIO_SetOutputPin(hfram->CS_Port, hfram->CS_Pin);
            return status;
        }
    }

    LL_GPIO_SetOutputPin(hfram->CS_Port, hfram->CS_Pin);

    return status;
}

Std_ReturnType FRAM_SPI_ReadID(FRAM_SPI_HandleTypeDef *hfram, uint8_t *pID, uint16_t len)
{
    Std_ReturnType status;

    LL_GPIO_ResetOutputPin(hfram->CS_Port, hfram->CS_Pin);

    status = SPI_Driver_Write(hfram->SPIx, FRAM_RDID);
    if(status != E_OK) {
        LL_GPIO_SetOutputPin(hfram->CS_Port, hfram->CS_Pin);
        return status;
    }

    for (int i = 0; i < len; i++)
    {
        status = SPI_Driver_Transmit8(hfram->SPIx, 0x00, &pID[i]);
        if(status != E_OK) {
            LL_GPIO_SetOutputPin(hfram->CS_Port, hfram->CS_Pin);
            return status;
        }
    }

    LL_GPIO_SetOutputPin(hfram->CS_Port, hfram->CS_Pin);

    return status;
}
