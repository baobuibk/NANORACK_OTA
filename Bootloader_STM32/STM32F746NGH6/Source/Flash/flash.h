/*
 * flash.h
 *
 *  Created on: Feb 27, 2025
 *      Author: thanh
 */

#ifndef FLASH_FLASH_H_
#define FLASH_FLASH_H_

#include <stdint.h>



typedef enum {
    FLASH_OK = 0x00,
    FLASH_TIMEOUT = 0x01,
    FLASH_ERROR = 0x02
} FLASH_StatusTypeDef;

void FLASH_Unlock(void);
void FLASH_Lock(void);
FLASH_StatusTypeDef FLASH_Write_64Bit(uint32_t address, uint8_t *data);
FLASH_StatusTypeDef FLASH_Erase_Sectors(uint8_t bank, uint8_t sector);
FLASH_StatusTypeDef FLASH_Erase_All(uint8_t bank);
FLASH_StatusTypeDef FLASH_Erase_Both_Banks(void);
FLASH_StatusTypeDef FLASH_Write_Data(uint32_t address, uint8_t *data, uint32_t size);

#endif /* FLASH_FLASH_H_ */
