/*
 * spi_slave.h
 *
 *  Created on: Apr 17, 2025
 *      Author: CAO HIEU
 */

#ifndef M3_DEVICES_SPI_SLAVEOFCM4_SPI_SLAVE_H_
#define M3_DEVICES_SPI_SLAVEOFCM4_SPI_SLAVE_H_

#include "utils.h"
#include "gpio_state.h"
#include "stdbool.h"
#include "stdint.h"

typedef enum {
	SPI_TRANSFER_WAIT,
	SPI_TRANSFER_COMPLETE,
	SPI_TRANSFER_ERROR
} SPI_TransferState_t;

typedef struct {
	uint8_t type; // Type of data collection (0, 1, 2, 3)
	uint32_t sample; // Number of samples (1 to 100,000)
	uint32_t data_size; // Total data size in bytes (sample \* 2)
	uint16_t crc; // CRC16-XMODEM of the data
	_Bool is_valid; // Flag indicating if context is valid
} DataProcessContext_t;

typedef struct {
	SPI_TransferState_t transfer_state;
	DataProcessContext_t data_context;
	_Bool is_initialized;
} SPI_SlaveDevice_t;

SPI_SlaveDevice_t* SPI_SlaveDevice_GetHandle(void);
Std_ReturnType SPI_SlaveDevice_Init(void);
Std_ReturnType SPI_SlaveDevice_CollectData(uint8_t type, uint32_t sample, uint32_t data_addr);
Std_ReturnType SPI_SlaveDevice_GetDataInfo(DataProcessContext_t *context);
Std_ReturnType SPI_SlaveDevice_ResetDMA(uint32_t data_addr, uint32_t data_size);
Std_ReturnType SPI_SlaveDevice_Disable(void);
SPI_TransferState_t SPI_SlaveDevice_GetTransferState(void);
void SPI_SlaveDevice_SetTransferState(SPI_TransferState_t state);
toCM4_State_t SPI_SlaveDevice_GetCM4State(void);

#endif /* M3_DEVICES_SPI_SLAVEOFCM4_SPI_SLAVE_H_ */
