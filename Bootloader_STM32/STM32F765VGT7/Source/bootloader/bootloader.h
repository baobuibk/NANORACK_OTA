#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include "main.h"
#include <math.h>
#include <stm32f7xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "UART.h"
#include <stm32f7xx_hal.h>
#include <stm32f7xx_ll_bus.h>



#define BLD_TIMEOUT					20000		// Timeout cho bootloader ở trạng thái reset do request là 10s

#define BOOTLOADER_MEM_BASE       	0x08000000U  // Sector 0, 1, 2 (bootloader, 96 KB)
#define METADATA_MEM_BASE         	0x08018000U  // Sector 3 (metadata, 32 KB)

#define FIRMWARE1_MEM_BASE        	0x08020000U  // Sector 4, 5 (firmware1)
#define FIRMWARE2_MEM_BASE        	0x08080000U  // Sector 6 (firmware2)
#define FIRMWARE3_MEM_BASE        	0x080C0000U  // Sector 7 (firmware3)

#define FIRMWARE2_MEM_OFFSET		(uint32_t)(FIRMWARE2_MEM_BASE - FIRMWARE1_MEM_BASE)
#define FIRMWARE3_MEM_OFFSET		(uint32_t)(FIRMWARE3_MEM_BASE - FIRMWARE1_MEM_BASE)

#define FIRMWARE_SIZE               (384 * 1024) // Kích thước firmware (384 KB)
#define FLASH_END_ADDRESS			0x080FFFFFU
// Sector cho firmware
#define FIRMWARE_SECTOR_ALL         4  		// Sector 4
#define FIRMWARE_NUM_SECTORS_ALL    4		// Số sector cho 3 firmware (4, 5, 6, 7)

#define FIRMWARE1_SECTOR            4  		// Sector 4
#define FIRMWARE1_NUM_SECTORS       2		// Số sector cho firmware1 (4, 5)

#define FIRMWARE2_SECTOR            6 		// Sector 6
#define FIRMWARE2_NUM_SECTORS       1		// Số sector cho firmware1 (6)

#define FIRMWARE3_SECTOR            7  		// Sector 7
#define FIRMWARE3_NUM_SECTORS       1		// Số sector cho firmware1 (7)


#define BL_HOST_BUFFER_RX_LENGTH         100

#define CBL_GET_CID_CMD                  0x10
#define CBL_GET_RDP_STATUS_CMD           0x11
#define CBL_GO_TO_ADDR_CMD               0x12
#define CBL_FLASH_ERASE_CMD              0x13
#define CBL_MEM_WRITE_CMD                0x14
#define CBL_CHECK_CONNECTION             0x15
#define CBL_GET_VERSION                  0x16
#define CBL_SET_VERSION                  0x17

#define CRC_TYPE_SIZE_BYTE               4
#define CBL_SEND_NACK                    0xAB

#define CBL_FLASH_MAX_SECTOR_NUMBER      8  // total sector
#define CBL_FLASH_MASS_ERASE             0xFF

#define FOTA_SUCCESS               			0
#define FOTA_FAILED                 		1

#define CBL_ROP_LEVEL_0                  0
#define CBL_ROP_LEVEL_1                  1
#define CBL_ROP_LEVEL_2                  2

typedef struct _s_firmware_info_
{
    bool fota_rqt;
    uint32_t address;      // Địa chỉ firmware
    uint32_t length;       // Độ dài firmware
    uint32_t crc;          // CRC cho firmware
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
} s_firmware_info;

typedef void (*pMainApp)(void);
typedef void (*Jump_Ptr)(void);

extern volatile uint32_t boot_timeout;

void BL_UART_Fetch_Host_Command(void*);
uint8_t Jump_To_App(uint32_t app_address);
void Bootloader_Check_Reset_Reason(void);
void Bootloader_Check_Timeout(void*);

#endif /* BOOTLOADER_H_ */
