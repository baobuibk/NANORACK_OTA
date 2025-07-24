/*
 * spi_master.h
 *
 *  Created on: Apr 18, 2025
 *      Author: CAO HIEU
 */

#ifndef M3_DEVICES_SPI_MASTEROFEXP_SPI_MASTER_H_
#define M3_DEVICES_SPI_MASTEROFEXP_SPI_MASTER_H_

#include "utils.h"
#include "gpio_state.h"
#include "stdbool.h"
#include "stdint.h"
#include "stm32h7xx_ll_spi.h"

typedef enum {
	SPI_MASTER_TRANSFER_IDLE,
    SPI_MASTER_TRANSFER_BUSY,
    SPI_MASTER_TRANSFER_COMPLETE,
    SPI_MASTER_TRANSFER_ERROR
} SPI_MasterTransferState_t;

typedef struct {
	SPI_TypeDef *SPIx;
    GPIO_TypeDef *CS_Port;
    uint16_t CS_Pin;
    SPI_MasterTransferState_t transfer_state;
    _Bool is_initialized;
} SPI_MasterDevice_t;

SPI_MasterDevice_t* SPI_MasterDevice_GetHandle(void);
Std_ReturnType SPI_MasterDevice_Init(SPI_TypeDef *SPIx, GPIO_TypeDef *CS_Port, uint16_t CS_Pin);
Std_ReturnType SPI_MasterDevice_ReadDMA(uint32_t data_addr, uint32_t size);
Std_ReturnType SPI_MasterDevice_Disable(void);
SPI_MasterTransferState_t SPI_MasterDevice_GetTransferState(void);
void SPI_MasterDevice_SetTransferState(SPI_MasterTransferState_t state);
toCM4_State_t SPI_MasterDevice_GetCM4State(void);
void SPIMaster_IRQHandler(void);

#endif /* M3_DEVICES_SPI_MASTEROFEXP_SPI_MASTER_H_ */
