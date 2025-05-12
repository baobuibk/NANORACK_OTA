#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include "main.h"
#include <stm32h7xx.h>
#include <stm32h745xx.h>
#include "core_cm4.h"
#include <stdbool.h>

// Định nghĩa địa chỉ bộ nhớ cho STM32H745ZIT3
#define BOOTLOADER_BANK1_BASE       0x08000000U  // Sector 0 Bank 1 (bootloader, 128 KB)
#define METADATA_BANK1_BASE         0x08020000U  // Sector 1 Bank 1 (metadata, 128 KB)
#define FIRMWARE_BANK1_BASE         0x08040000U  // Sector 2 Bank 1 (firmware)
#define BOOTLOADER_BANK2_BASE       0x08100000U  // Sector 0 Bank 2 (bootloader, 128 KB)
#define METADATA_BANK2_BASE         0x08120000U  // Sector 1 Bank 2 (metadata, 128 KB)
#define FIRMWARE_BANK2_BASE         0x08140000U  // Sector 2 Bank 2 (firmware)

typedef enum
{
    BL_NACK = 0,
    BL_OK
} BL_Status;

typedef void (*pMainApp)(void);
typedef void (*Jump_Ptr)(void);

bool Jump_To_App(void);

#endif /* BOOTLOADER_H_ */
