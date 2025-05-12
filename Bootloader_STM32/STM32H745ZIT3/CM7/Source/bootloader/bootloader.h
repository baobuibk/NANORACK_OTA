#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include "main.h"
#include <stm32h7xx.h>
#include <stm32h745xx.h>
#include "flash.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "UART.h"
#include <stm32h7xx_hal.h>
#include <stm32h7xx_ll_bus.h>



#define BL_TIMEOUT					10000		// Timeout cho bootloader là 10s
// Định nghĩa địa chỉ bộ nhớ cho STM32H745ZIT3
#define BOOTLOADER_BANK1_BASE       0x08000000U  // Sector 0 Bank 1 (bootloader, 128 KB)
#define METADATA_BANK1_BASE         0x08020000U  // Sector 1 Bank 1 (metadata, 128 KB)
#define FIRMWARE_BANK1_BASE         0x08040000U  // Sector 2 Bank 1 (firmware)
#define BOOTLOADER_BANK2_BASE       0x08100000U  // Sector 0 Bank 2 (bootloader, 128 KB)
#define METADATA_BANK2_BASE         0x08120000U  // Sector 1 Bank 2 (metadata, 128 KB)
#define FIRMWARE_BANK2_BASE         0x08140000U  // Sector 2 Bank 2 (firmware)

#define FIRMWARE_SIZE               (1024 * 1024) // Kích thước firmware (512 KB)
#define METADATA_SIZE               256           // Kích thước metadata (byte, không phải KB)

// Sector cho firmware
#define FIRMWARE1_SECTOR            2  // Sector 2 Bank 1
#define FIRMWARE2_SECTOR            10  // Sector 2 Bank 2
#define FIRMWARE_NUM_SECTORS        4              // Số sector cho firmware (2, 3, 4, 5)

// Các định nghĩa khác giữ nguyên
#define BL_HOST_BUFFER_RX_LENGTH         100
#define BL_ENABLE_UART_DEBUG_MESSAGE     0x00
#define BL_ENABLE_SPI_DEBUG_MESSAGE      0x01
#define BL_ENABLE_CAN_DEBUG_MESSAGE      0x02
#define BL_DEBUG_METHOD                  (BL_ENABLE_UART_DEBUG_MESSAGE)

#define CBL_GET_CID_CMD                  0x10
#define CBL_GET_RDP_STATUS_CMD           0x11
#define CBL_GO_TO_ADDR_CMD               0x12
#define CBL_FLASH_ERASE_CMD              0x13
#define CBL_MEM_WRITE_CMD                0x14
#define CBL_RESET_CHIP                   0x15
#define CBL_GET_VERSION                  0x16
#define CBL_SET_VERSION                  0x17
#define CBL_CHECK_CONNECTION			 0x62

#define CRC_TYPE_SIZE_BYTE               4
#define CRC_VERIFICATION_FAILED          0x00
#define CRC_VERIFICATION_PASSED          0x01
#define CBL_SEND_NACK                    0xAB

#define ADDRESS_IS_INVALID               0x00
#define ADDRESS_IS_VALID                 0x01

#define STM32H745_FLASH_SIZE             (2048 * 1024) // 2 MB Flash
#define STM32H745_FLASH_END              (0x08000000 + STM32H745_FLASH_SIZE)

#define CBL_FLASH_MAX_SECTOR_NUMBER      16  // Tổng số sector (8 sector mỗi ngân hàng)
#define CBL_FLASH_MASS_ERASE             0xFF

#define INVALID_SECTOR_NUMBER            0x00
#define VALID_SECTOR_NUMBER              0x01

#define UNSUCCESSFUL_ERASE               0x00
#define SUCCESSFUL_ERASE                 0x01

#define HAL_SUCCESSFUL_ERASE             0xFFFFFFFFU

#define FLASH_PAYLOAD_WRITE_FAILED       0x00
#define FLASH_PAYLOAD_WRITE_PASSED       0x01

#define FLASH_LOCK_WRITE_FAILED          0x00
#define FLASH_LOCK_WRITE_PASSED          0x01

#define ROP_LEVEL_READ_INVALID           0x00
#define ROP_LEVEL_READ_VALID             0x01

#define ROP_LEVEL_CHANGE_INVALID         0x00
#define ROP_LEVEL_CHANGE_VALID           0x01

#define CBL_ROP_LEVEL_0                  0x00
#define CBL_ROP_LEVEL_1                  0x01
#define CBL_ROP_LEVEL_2                  0x02

typedef enum
{
    BL_NACK = 0,
    BL_OK
} BL_Status;

typedef void (*pMainApp)(void);
typedef void (*Jump_Ptr)(void);

extern volatile uint32_t boot_timeout;
extern bool check_connection;

void BL_UART_Fetch_Host_Command(void*);
void BL_Print_Message(char *format, ...);
void Bootloader_Check_Available(void*);
void Wait_For_Request(void);
bool Jump_To_App(uint32_t app_address); // Chỉ cần 1 địa chỉ cho Cortex-M7

#endif /* BOOTLOADER_H_ */
