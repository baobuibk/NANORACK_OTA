/*
 * bsp_system.c
 *
 *  Created on: Jul 21, 2025
 *      Author: DELL
 */


#include "bsp_system.h"

/**
 * Ghi metadata của một firmware vào Flash.
 * @param fw: Con trỏ tới thông tin firmware (s_firmware_info).
 * @param fw_number: Số thứ tự firmware (1 cho bank 1, 2 cho bank 2).
 * @return FLASH_PAYLOAD_WRITE_PASSED nếu thành công, FLASH_PAYLOAD_WRITE_FAILED nếu lỗi hoặc đầu vào không hợp lệ.
 */
static uint8_t Flash_Write_Metadata(s_firmware_info* fw_info, uint8_t fw_number)
{
	if((fw_info == NULL)||(fw_number < 1)||(fw_number > 2))	return FOTA_FAILED;

	uint32_t metadata_addr = (fw_number == 1) ? METADATA_CORE1_MEM_BASE : METADATA_CORE2_MEM_BASE;
	uint32_t bank = (fw_number == 1) ? FLASH_BANK_1 : FLASH_BANK_2;
    uint8_t data[32] __attribute__((aligned(32)));
    uint32_t fw_info_size = sizeof(s_firmware_info);
    memset(data, 0xFF, sizeof(data));
    memcpy(data, fw_info, fw_info_size);

    HAL_FLASH_Unlock();

	FLASH_EraseInitTypeDef EraseInitStruct = {0};
	uint32_t SectorError = 0;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.Banks = bank;
	EraseInitStruct.Sector = 1; 						// Metadata ở Sector 1
	EraseInitStruct.NbSectors = 1;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
	{
		HAL_FLASH_Lock();
		return FOTA_FAILED;
	}

	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, metadata_addr, (uint32_t)data) != HAL_OK)
	{
		HAL_FLASH_Lock();
		return FOTA_FAILED;
	}
    HAL_FLASH_Lock();
    return FOTA_SUCCESS;
}

/**
 * Đọc metadata của một firmware từ Flash.
 * @param fw_info: Con trỏ tới cấu trúc s_firmware_info để lưu thông tin metadata.
 * @param fw_number: Số thứ tự firmware (1 cho bank 1, 2 cho bank 2).
 * @return FLASH_PAYLOAD_WRITE_PASSED nếu đọc thành công và metadata hợp lệ,
 *         FLASH_PAYLOAD_WRITE_FAILED nếu fw_number không hợp lệ hoặc metadata không hợp lệ.
 */
static uint8_t Flash_Read_Metadata(s_firmware_info* fw_info, uint8_t fw_number)
{
	if((fw_number < 1)||(fw_number > 2)) return FOTA_FAILED;

	uint32_t metadata_addr = (fw_number == 1) ? METADATA_CORE1_MEM_BASE : METADATA_CORE2_MEM_BASE;

    uint8_t data[32] __attribute__((aligned(32)));
    uint32_t fw_info_size = sizeof(s_firmware_info);

    memcpy(data, (void*)metadata_addr, sizeof(data));
    memcpy(fw_info, data, fw_info_size);

    if((fw_info->address == 0xffffffff)||
	(fw_info->length == 0xffffffff)||
	(fw_info->crc == 0xffffffff))
    	return FOTA_FAILED;
    return FOTA_SUCCESS;
}

uint8_t System_On_Bootloader_Reset(void)
{
	s_firmware_info fw_info;
	if (Flash_Read_Metadata(&fw_info, 1) != FOTA_SUCCESS) return FOTA_FAILED;
	fw_info.fota_rqt = true;
	if (Flash_Write_Metadata(&fw_info, 1) != FOTA_SUCCESS) return FOTA_FAILED;
    __disable_irq();
    __HAL_RCC_CLEAR_RESET_FLAGS();
    HAL_DeInit();
    NVIC_SystemReset();

	return FOTA_SUCCESS;
}



