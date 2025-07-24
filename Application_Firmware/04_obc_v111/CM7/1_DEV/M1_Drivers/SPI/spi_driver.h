/*
 * spi_driver.h
 *
 *  Created on: Feb 26, 2025
 *      Author: CAO HIEU
 */

#ifndef M1_DRIVERS_SPI_SPI_DRIVER_H_
#define M1_DRIVERS_SPI_SPI_DRIVER_H_

#include "stm32h7xx_ll_spi.h"
#include "utils.h"

Std_ReturnType SPI_Driver_Transmit8(SPI_TypeDef *SPIx, uint8_t data, uint8_t *receivedData);
Std_ReturnType SPI_Driver_Transmit16(SPI_TypeDef *SPIx, uint16_t data, uint16_t *receivedData);
Std_ReturnType SPI_Driver_Write(SPI_TypeDef *SPIx, uint8_t data);
Std_ReturnType SPI_Driver_TransmitArray(SPI_TypeDef *SPIx, uint8_t *data, uint16_t size);

#endif /* M1_DRIVERS_SPI_SPI_DRIVER_H_ */
