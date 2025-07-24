/*
 * bsp_system.h
 *
 *  Created on: Jul 21, 2025
 *      Author: DELL
 */

#ifndef BSUPPORT_BSP_BSP_SYSTEM_BSP_SYSTEM_H_
#define BSUPPORT_BSP_BSP_SYSTEM_BSP_SYSTEM_H_

#include <math.h>
#include <stm32h7xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stm32h7xx_hal.h>
#include <stm32h7xx_ll_bus.h>

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

#define FOTA_SUCCESS               			0
#define FOTA_FAILED                 		1

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


uint8_t System_On_Bootloader_Reset(void);

#endif /* BSUPPORT_BSP_BSP_SYSTEM_BSP_SYSTEM_H_ */
