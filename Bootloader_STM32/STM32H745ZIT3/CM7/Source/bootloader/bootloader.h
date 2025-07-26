#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include "main.h"
#include <math.h>
#include <stm32h7xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "UART.h"
#include <stm32h7xx_hal.h>
#include <stm32h7xx_ll_bus.h>



#define BLD_TIMEOUT					30000			// Timeout cho bootloader ở trạng thái reset do request là 30s
#define BLD_TIMEOUT_NORMAL			5000
// Bank 1
#define BOOTLOADER_CORE1_MEM_BASE   0x08000000U  	// Sector 0(bootloader, 128 KB)
#define METADATA_CORE1_MEM_BASE     0x08020000U  	// Sector 1 (metadata, 128 KB)

#define FIRMWARE1_CORE1_MEM_BASE	0x08040000U  	// Sector 2, 3, 4 (firmware1)
#define FIRMWARE2_CORE1_MEM_BASE	0x080A0000U  	// Sector 5, 6, 7 (firmware2)

// Bank 2
#define BOOTLOADER_CORE2_MEM_BASE   0x08100000U  	// Sector 0(bootloader, 128 KB)
#define METADATA_CORE2_MEM_BASE     0x08120000U  	// Sector 1 (metadata, 128 KB)

#define FIRMWARE1_CORE2_MEM_BASE	0x08140000U  	// Sector 2, 3, 4 (firmware1)
#define FIRMWARE2_CORE2_MEM_BASE	0x081A0000U  	// Sector 5, 6, 7 (firmware2)

// Offset base
#define FIRMWARE_MEM_OFFSET		(uint32_t)(FIRMWARE2_CORE1_MEM_BASE - FIRMWARE1_CORE1_MEM_BASE)

#define FIRMWARE_SIZE               (384 * 1024) // Kích thước firmware (384 KB)
#define FLASH_END_ADDRESS			0x081FFFFFU

// Sector firmware CORE1
#define FIRMWARE_CORE1_SECTOR_ALL         2  		// Sector 2
#define FIRMWARE_CORE1_NUM_SECTORS_ALL    6			// Số sector cho 2 firmware CORE1 (2, 3, 4, 5, 6, 7)

#define FIRMWARE1_CORE1_SECTOR            2  		// Sector 2
#define FIRMWARE1_CORE1_NUM_SECTORS       3			// Số sector cho firmware1 CORE1 (2, 3, 4)

#define FIRMWARE2_CORE1_SECTOR            5  		// Sector 5
#define FIRMWARE2_CORE1_NUM_SECTORS       3			// Số sector cho firmware1 CORE1 (5, 6, 7)

// Sector firmware CORE2
#define FIRMWARE_CORE2_SECTOR_ALL         2  		// Sector 2
#define FIRMWARE_CORE2_NUM_SECTORS_ALL    6			// Số sector cho 2 firmware CORE2 (2, 3, 4, 5, 6, 7)

#define FIRMWARE1_CORE2_SECTOR            2  		// Sector 2
#define FIRMWARE1_CORE2_NUM_SECTORS       3			// Số sector cho firmware1 CORE2 (2, 3, 4)

#define FIRMWARE2_CORE2_SECTOR            5  		// Sector 5
#define FIRMWARE2_CORE2_NUM_SECTORS       3			// Số sector cho firmware1 CORE2 (5, 6, 7)


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


// Reset_cause
#define WDG_NO_INIT_VARS_MAGIC 		0xdeaddead
#define RESET_CAUSE_NORMAL			0
#define RESET_CAUSE_BOOTLOADER		1


typedef struct _s_firmware_info_
{
    uint32_t address;      // Địa chỉ firmware
    uint32_t length;       // Độ dài firmware
    uint32_t crc;          // CRC cho firmware
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
} s_firmware_info;

typedef struct
{
    uint32_t magic;
    uint32_t reset_cause;
    uint32_t reset_wdg_id;
}wdg_no_init_vars;

typedef void (*pMainApp)(void);
typedef void (*Jump_Ptr)(void);

extern volatile uint32_t boot_timeout;

void BL_UART_Fetch_Host_Command(void*);
uint8_t Jump_To_App(uint32_t app_address);
//void Bootloader_Check_Reset_Reason(void);
void Bootloader_Check_Timeout(void*);
void update_no_init_vars(uint32_t reset_cause);
void validate_no_init_vars(void);
#endif /* BOOTLOADER_H_ */
