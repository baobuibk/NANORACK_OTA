/*
 * spi_fram.h
 *
 *  Created on: Feb 26, 2025
 *      Author: CAO HIEU
 */

#ifndef M3_DEVICES_SPI_FRAM_FRAM_SPI_H_
#define M3_DEVICES_SPI_FRAM_FRAM_SPI_H_

#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_spi.h"
#include "utils.h"
#include "stdint.h"

typedef struct {
    SPI_TypeDef *SPIx;          //!< SPI instance (e.g., SPI4)
    GPIO_TypeDef *CS_Port;      //!< GPIO port for the Chip Select pin
    uint16_t CS_Pin;            //!< GPIO pin for Chip Select
} FRAM_SPI_HandleTypeDef;

FRAM_SPI_HandleTypeDef* FRAM_SPI_GetHandle(void);

/**
 * @brief Initializes the FRAM driver over SPI.
 * @param SPIx: SPI instance (e.g., SPI4)
 * @param CS_Port: GPIO port for the CS pin
 * @param CS_Pin: GPIO pin for the CS pin
 * @return Pointer to the FRAM handle
 */
void FRAM_SPI_Driver_Init(SPI_TypeDef *SPIx, GPIO_TypeDef *CS_Port, uint16_t CS_Pin);

/**
 * @brief Sends the Write Enable command to FRAM.
 * @param hfram: Pointer to the FRAM handle
 * @return Communication status (E_OK if successful)
 */
Std_ReturnType FRAM_SPI_WriteEnable(FRAM_SPI_HandleTypeDef *hfram);

/**
 * @brief Sends the Write Disable command to FRAM.
 * @param hfram: Pointer to the FRAM handle
 * @return Communication status (E_OK if successful)
 */
Std_ReturnType FRAM_SPI_WriteDisable(FRAM_SPI_HandleTypeDef *hfram);

/**
 * @brief Writes data to FRAM memory.
 * @param hfram: Pointer to the FRAM handle
 * @param addr: 24-bit memory address
 * @param pData: Pointer to the buffer containing data to write
 * @param len: Number of bytes to write
 * @return Communication status (E_OK if successful)
 */
Std_ReturnType FRAM_SPI_WriteMem(FRAM_SPI_HandleTypeDef *hfram, uint32_t addr, uint8_t *pData, uint16_t len);

/**
 * @brief Reads data from FRAM memory.
 * @param hfram: Pointer to the FRAM handle
 * @param addr: 24-bit memory address
 * @param pData: Pointer to the buffer to store the read data
 * @param len: Number of bytes to read
 * @return Communication status (E_OK if successful)
 */
Std_ReturnType FRAM_SPI_ReadMem(FRAM_SPI_HandleTypeDef *hfram, uint32_t addr, uint8_t *pData, uint16_t len);

/**
 * @brief Reads the Device ID from FRAM.
 * @param hfram: Pointer to the FRAM handle
 * @param pID: Pointer to the buffer to store the ID (caller decides the length)
 * @param len: Number of bytes to read for the Device ID
 * @return Communication status (E_OK if successful)
 */
Std_ReturnType FRAM_SPI_ReadID(FRAM_SPI_HandleTypeDef *hfram, uint8_t *pID, uint16_t len);


#endif /* M3_DEVICES_SPI_FRAM_FRAM_SPI_H_ */
