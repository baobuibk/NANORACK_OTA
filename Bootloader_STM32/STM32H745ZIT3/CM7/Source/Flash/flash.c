#include "stm32h7xx_hal.h"

// Định nghĩa kiểu trả về
typedef HAL_StatusTypeDef FLASH_StatusTypeDef;

/**
 * @brief  Mở khóa bộ nhớ Flash để cho phép ghi/xóa
 */
void FLASH_Unlock(void)
{
    HAL_FLASH_Unlock();
}

/**
 * @brief  Khóa bộ nhớ Flash để ngăn ghi/xóa
 */
void FLASH_Lock(void)
{
    HAL_FLASH_Lock();
}

/**
 * @brief  Ghi dữ liệu 64-bit (8 byte) vào địa chỉ Flash được chỉ định
 * @param  address: Địa chỉ Flash để ghi (phải được căn chỉnh 8 byte)
 * @param  data: Con trỏ đến dữ liệu cần ghi (8 byte)
 * @retval FLASH_StatusTypeDef: HAL_OK nếu thành công, HAL_ERROR nếu thất bại
 */
FLASH_StatusTypeDef FLASH_Write_64Bit(uint32_t address, uint8_t *data)
{
    FLASH_StatusTypeDef status = HAL_OK;
    uint64_t data64 = *(uint64_t *)data; // Chuyển đổi dữ liệu thành 64-bit

    // Kiểm tra địa chỉ có hợp lệ và căn chỉnh 8 byte
    if ((address < FLASH_BASE) || (address > (FLASH_BASE + 0x100000 - 8)) || (address % 8 != 0))
    {
        return HAL_ERROR;
    }

    // Ghi dữ liệu 64-bit
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, data64);

    return status;
}

/**
 * @brief  Xóa một sector của Flash
 * @param  bank: Ngân hàng Flash (chỉ chấp nhận 1 vì STM32F746 không có dual-bank)
 * @param  sector: Số sector (0-7)
 * @retval FLASH_StatusTypeDef: HAL_OK nếu thành công, HAL_ERROR nếu thất bại
 */
FLASH_StatusTypeDef FLASH_Erase_Sectors(uint8_t bank, uint8_t sector)
{
    FLASH_StatusTypeDef status = HAL_OK;
    FLASH_EraseInitTypeDef eraseInit = {0};
    uint32_t sectorError = 0;

    // Kiểm tra bank và sector hợp lệ
    if (bank != 1 || sector > 7)
    {
        return HAL_ERROR;
    }

    // Cấu hình thông tin xóa
    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.Sector = sector;
    eraseInit.NbSectors = 1;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3; // Giả sử VDD = 2.7V-3.6V

    // Xóa sector
    status = HAL_FLASHEx_Erase(&eraseInit, &sectorError);
    if (sectorError != 0xFFFFFFFF) // Kiểm tra lỗi sector
    {
        status = HAL_ERROR;
    }

    return status;
}

/**
 * @brief  Xóa toàn bộ Flash của một bank
 * @param  bank: Ngân hàng Flash (chỉ chấp nhận 1 vì STM32F746 không có dual-bank)
 * @retval FLASH_StatusTypeDef: HAL_OK nếu thành công, HAL_ERROR nếu thất bại
 */
FLASH_StatusTypeDef FLASH_Erase_All(uint8_t bank)
{
    FLASH_StatusTypeDef status = HAL_OK;
    FLASH_EraseInitTypeDef eraseInit = {0};
    uint32_t sectorError = 0;

    // Kiểm tra bank hợp lệ
    if (bank != 1)
    {
        return HAL_ERROR;
    }

    // Cấu hình xóa toàn bộ Flash
    eraseInit.TypeErase = FLASH_TYPEERASE_MASSERASE;
    eraseInit.Banks = FLASH_BANK_1;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    // Xóa toàn bộ bank
    status = HAL_FLASHEx_Erase(&eraseInit, &sectorError);
    if (sectorError != 0xFFFFFFFF)
    {
        status = HAL_ERROR;
    }

    return status;
}

/**
 * @brief  Xóa cả hai bank của Flash (chỉ áp dụng bank 1 cho STM32F746)
 * @retval FLASH_StatusTypeDef: HAL_OK nếu thành công, HAL_ERROR nếu thất bại
 */
FLASH_StatusTypeDef FLASH_Erase_Both_Banks(void)
{
    // STM32F746 chỉ có 1 bank, nên hàm này tương đương FLASH_Erase_All(1)
    return FLASH_Erase_All(1);
}

/**
 * @brief  Ghi dữ liệu có kích thước tùy ý vào Flash
 * @param  address: Địa chỉ Flash để ghi (phải căn chỉnh 4 byte cho từ 32-bit)
 * @param  data: Con trỏ đến dữ liệu cần ghi
 * @param  size: Kích thước dữ liệu (byte)
 * @retval FLASH_StatusTypeDef: HAL_OK nếu thành công, HAL_ERROR nếu thất bại
 */
FLASH_StatusTypeDef FLASH_Write_Data(uint32_t address, uint8_t *data, uint32_t size)
{
    FLASH_StatusTypeDef status = HAL_OK;
    uint32_t i = 0;

    // Kiểm tra địa chỉ và kích thước hợp lệ
    if ((address < FLASH_BASE) || (address + size > FLASH_BASE + 0x100000) || (data == NULL))
    {
        return HAL_ERROR;
    }

    // Ghi từng từ 32-bit (4 byte) để tối ưu
    while (i < size)
    {
        uint32_t data32 = 0;
        uint32_t bytesToWrite = (size - i >= 4) ? 4 : (size - i);

        // Tạo dữ liệu 32-bit từ mảng byte
        for (uint32_t j = 0; j < bytesToWrite; j++)
        {
            data32 |= (uint32_t)(data[i + j]) << (j * 8);
        }

        // Ghi từ 32-bit
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address + i, data32);
        if (status != HAL_OK)
        {
            return status;
        }

        i += 4;
    }

    return HAL_OK;
}
