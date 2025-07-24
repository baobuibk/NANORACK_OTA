/************************************             Includes               ************************************/
#include "bootloader.h"


extern usart_meta_t UART7_meta;
extern usart_meta_t *p_UART7_meta;
s_firmware_info Temp_Firmware;
static uint32_t Address_to_write;
volatile uint32_t boot_timeout = 0;

/************************************    Static Functions Declarations  ************************************/
static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer);
static void Bootloader_Jump_To_User_App(void);
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer);
static void Bootloader_Memory_Write(uint8_t *Host_Buffer);
static uint8_t Firmware_Check_Available(void);
static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC);
static void Bootloader_Send_NACK(void);
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len);
static uint8_t Perform_Flash_Erase(uint8_t Sector_Index, uint8_t Number_Of_Sectors);
static void Set_Firmware_Version(uint8_t *Host_Buffer);
static void Get_Firmware_Version(uint8_t *Host_Buffer);
static uint32_t Firmware_CRC_Verification(uint32_t start_address, uint32_t length);
static uint8_t Flash_Write_Metadata(s_firmware_info* fw_info);
static uint8_t Flash_Read_Metadata(s_firmware_info* fw_info);
static uint8_t Flash_CopyData(uint32_t src_addr, uint32_t dst_addr, uint32_t length);
static void Bootloader_check_connection(void);

/************************************    Global Variables Definitions     ************************************/
static uint8_t BL_Host_Buffer[150];
uint8_t frame_index = 0;
uint8_t frame_length = 0;
bool receiving_frame = false;
uint16_t frame_timeout = 0;

/************************************ Software Interfaces Implementations ************************************/
void BL_UART_Fetch_Host_Command(void*)
{
    uint8_t data;
    if (receiving_frame)
    {
        if (frame_timeout++ > 500)
        {
            receiving_frame = false;
            frame_length = 0;
            frame_index = 0;
            frame_timeout = 0;
        }
    }
    while (!rbuffer_empty(&p_UART7_meta->rb_rx))
    {
        data = rbuffer_remove(&p_UART7_meta->rb_rx);
        if (!receiving_frame)
        {
            // Nhận byte đầu tiên (FRAME LENGTH)
            frame_timeout = 0;
            frame_length = data;
            if (frame_length > 0 && frame_length < 255)
            {
                BL_Host_Buffer[0] = frame_length;
                frame_index = 1;
                receiving_frame = true;
            }
            else
            {
                // Nếu frame_length không hợp lệ, reset trạng thái
                frame_index = 0;
                receiving_frame = false;
            }
        }
        else
        {
            BL_Host_Buffer[frame_index++] = data;

            if (frame_index >= frame_length + 1)
            {
                receiving_frame = false;
                frame_length = 0;
                uint16_t Host_CMD_Packet_Len = 0;
                uint32_t Host_CRC32 = 0;
                /* Extract the CRC32 and packet length sent by the HOST */
                Host_CMD_Packet_Len = BL_Host_Buffer[0] + 1;
                Host_CRC32 = *((uint32_t*) ((BL_Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
                if (Bootloader_CRC_Verify((uint8_t*) &BL_Host_Buffer[0], Host_CMD_Packet_Len - 4, Host_CRC32) != FOTA_SUCCESS)
                {
                    Bootloader_Send_NACK();
                    return;
                }
                boot_timeout = BLD_TIMEOUT;
                switch (BL_Host_Buffer[1])
                {
					case CBL_GET_CID_CMD:
						Bootloader_Get_Chip_Identification_Number(BL_Host_Buffer);
						break;
					case CBL_GO_TO_ADDR_CMD:
						Bootloader_Jump_To_User_App();
						break;
					case CBL_FLASH_ERASE_CMD:
						Bootloader_Erase_Flash(BL_Host_Buffer);
						break;
					case CBL_MEM_WRITE_CMD:
						Bootloader_Memory_Write(BL_Host_Buffer);
						break;
					case CBL_SET_VERSION:
						Set_Firmware_Version(BL_Host_Buffer);
						break;
					case CBL_GET_VERSION:
						Get_Firmware_Version(BL_Host_Buffer);
						break;
					case CBL_CHECK_CONNECTION:
						Bootloader_check_connection();
					default:
						break;
                }
            }
        }
    }
}

/************************************    Static Functions Implementations  ************************************/

static void Bootloader_check_connection(void)
{
	uint8_t status[3] = {FOTA_SUCCESS, 'O', 'K'};
	Bootloader_Send_Data_To_Host((uint8_t*)status, 3);
}

void Bootloader_Check_Reset_Reason(void)
{
	s_firmware_info fw_info;
	if(Flash_Read_Metadata(&fw_info) != FOTA_SUCCESS)	return;
	if(fw_info.fota_rqt == false)		// reset due to hw, ...
	{
		Bootloader_Jump_To_User_App();
	}
	else	// else: reset due to host request to bootloader mode
	{
		// send frame to host: reset to bootloader done
		uint8_t status[3] = {FOTA_SUCCESS, 'O', 'K'};
		Bootloader_Send_Data_To_Host((uint8_t*)status, 3);
		boot_timeout = BLD_TIMEOUT;
	}

}

void Bootloader_Check_Timeout(void*)
{
	if(!boot_timeout)
	{
		boot_timeout = BLD_TIMEOUT;
		Firmware_Check_Available();
	}
}

/**
 * Lấy số nhận dạng chip và gửi về host
 * @param Host_Buffer: Buffer từ host (không dùng trong hàm này)
 */
static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer)
{
    uint16_t MCU_Identification_Number = 0;
    MCU_Identification_Number = (uint16_t)(DBGMCU->IDCODE & 0x00000FFF);
    Bootloader_Send_Data_To_Host((uint8_t*)&MCU_Identification_Number, 2);
}


/**
 * Nhảy đến ứng dụng người dùng
 * @param app_address: Địa chỉ bắt đầu của ứng dụng
 * @return true nếu nhảy thành công, false nếu thất bại
 */
uint8_t Jump_To_App(uint32_t app_address) {
    if (*((volatile uint32_t*)app_address) != 0xFFFFFFFF)
    {
        uint8_t status = FOTA_SUCCESS;

        Bootloader_Send_Data_To_Host((uint8_t*)&status, 1);
        HAL_Delay(1);

        while (!rbuffer_empty(&p_UART7_meta->rb_tx));
        __disable_irq();

        for (uint8_t i = 0; i < 8; i++)
        {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }

        LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_UART7);
        LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_UART7);

        if (SCB->CCR & SCB_CCR_DC_Msk)
        {
            SCB_CleanInvalidateDCache();
            SCB_DisableDCache();
        }
        if (SCB->CCR & SCB_CCR_IC_Msk)
        {
            SCB_InvalidateICache();
            SCB_DisableICache();
        }

        HAL_RCC_DeInit();
        HAL_DeInit();

        SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk | SCB_ICSR_PENDSTCLR_Msk;
        __DSB();
        __ISB();

        __set_MSP(*((volatile uint32_t*)app_address));
        SCB->VTOR = app_address;
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL = 0;

        __DSB();
        __ISB();

        uint32_t MainAppAddr = *((volatile uint32_t*)(app_address + 4));
        void (*reset_handler)(void) = (void(*)(void))MainAppAddr;

        __enable_irq();
        reset_handler();

        return FOTA_SUCCESS;
    }
    return FOTA_FAILED;
}

/**
 * Xử lý lệnh nhảy đến ứng dụng từ host
 * @param Host_Buffer: Buffer chứa địa chỉ ứng dụng
 */
static void Bootloader_Jump_To_User_App(void)
{
	uint8_t status = Firmware_Check_Available();
	if(status != FOTA_SUCCESS)
	{
		Bootloader_Send_Data_To_Host(&status, 1);
	}
}

/**
 * Thực hiện xóa Flash: xóa từng sector hoặc mass erase
 * @param Sector_Index: Số sector bắt đầu (0-15) hoặc CBL_FLASH_MASS_ERASE
 * @param Number_Of_Sectors: Số sector cần xóa
 * @return SUCCESSFUL_ERASE (0), UNSUCCESSFUL_ERASE (1), hoặc INVALID_Sector_Index (2)
 */
static uint8_t Perform_Flash_Erase(uint8_t Sector_Index, uint8_t Number_Of_Sectors)
{
    FLASH_EraseInitTypeDef EraseInitStruct = {0};
    uint32_t SectorError = 0;

    // Kiểm tra số sector hợp lệ
    if (Number_Of_Sectors > CBL_FLASH_MAX_SECTOR_NUMBER)
    {
        return FOTA_FAILED;
    }
    // Kiểm tra Sector_Index hợp lệ
    if ((Sector_Index != CBL_FLASH_MASS_ERASE) && (Sector_Index > 7))
    {
        return FOTA_FAILED;
    }

    HAL_FLASH_Unlock(); // Mở khóa flash
    // Xử lý mass erase
    if (Sector_Index == CBL_FLASH_MASS_ERASE)
    {
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_MASSERASE;
        //EraseInitStruct.Banks = FLASH_BANK_BOTH; // Xóa cả hai bank
        EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return FOTA_FAILED;
        }
        HAL_FLASH_Lock();
        return FOTA_SUCCESS;
    }

    // Xóa từng sector
//    if (Sector_Index < 8) 	EraseInitStruct.Banks = FLASH_BANK_1;
//    else					EraseInitStruct.Banks = FLASH_BANK_2;
    EraseInitStruct.Sector = Sector_Index;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.NbSectors = Number_Of_Sectors;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return FOTA_FAILED;
    }

    HAL_FLASH_Lock();
    return FOTA_SUCCESS;
}

/**
 * Xóa firmware trong Flash dựa trên lựa chọn từ host
 * @param Host_Buffer: Dữ liệu từ host, Host_Buffer[2] chọn firmware (1: Bank 1, 2: Bank 2)
 */
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer)
{
    uint8_t Erase_Status = FOTA_SUCCESS;
    s_firmware_info fw_info;
	fw_info.fota_rqt = false;
	fw_info.address = 0;
	fw_info.length = 0;
	fw_info.crc = 0;
	fw_info.version_major = 0;
	fw_info.version_minor = 0;
	fw_info.version_patch = 0;

    switch (Host_Buffer[2])
    {
        case 1: // Xóa Firmware 1 (Với F7 chỉ có 1 core với 3 fw dự phòng)
            Erase_Status = Perform_Flash_Erase(FIRMWARE1_SECTOR, FIRMWARE1_NUM_SECTORS);
            Erase_Status += Perform_Flash_Erase(FIRMWARE2_SECTOR, FIRMWARE2_NUM_SECTORS);
            Erase_Status += Perform_Flash_Erase(FIRMWARE3_SECTOR, FIRMWARE3_NUM_SECTORS);
            Erase_Status += Flash_Write_Metadata(&fw_info);
		break;

        default:
            Erase_Status = FOTA_FAILED; // Lựa chọn không hợp lệ
		break;
    }

    Bootloader_Send_Data_To_Host((uint8_t*)&Erase_Status, 1);

}

/**
 * Ghi payload vào Flash với độ dài bất kỳ, tự động chèn 0xFF nếu cần
 * @param Host_Payload: Dữ liệu cần ghi
 * @param Payload_Start_Address: Địa chỉ bắt đầu trong Flash
 * @param Payload_Len: Độ dài dữ liệu (byte)
 * @return FLASH_PAYLOAD_WRITE_PASSED (0) nếu thành công, FLASH_PAYLOAD_WRITE_FAILED (1) nếu lỗi
 */
static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_Payload, uint32_t Payload_Start_Address, uint16_t Payload_Len)
{
	if (Payload_Start_Address + Payload_Len > FLASH_END_ADDRESS) return FOTA_FAILED;
	if (Host_Payload == NULL || Payload_Len == 0)	return FOTA_FAILED;

	HAL_FLASH_Unlock(); // Mở khóa flash

    uint16_t i = 0;
    while (i < Payload_Len)
    {
        uint8_t buffer[4] __attribute__((aligned(4)));
        memset(buffer, 0xFF, sizeof(buffer));
        uint16_t bytes_to_write = (Payload_Len - i > 4) ? 4 : (Payload_Len - i);
        memcpy(buffer, &Host_Payload[i], bytes_to_write);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Payload_Start_Address + i, *(uint32_t *)buffer) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return FOTA_FAILED;
        }
        i += 4; // Tăng bước nhảy 4 byte
    }

    HAL_FLASH_Lock();
    return FOTA_SUCCESS;
}
/**
 * Ghi firmware vào Flash dựa trên lệnh từ host
 * @param Host_Buffer: Buffer chứa dữ liệu firmware
 * FRAME:
 * Byte 0: Packet Length
 * Byte 1: Command Code (0x14)
 * Byte 2: Firmware Number (0x01 hoặc 0x02)
 * Byte 3: Chunk size
 * Byte 4-5: Frame Index
 * Byte 6-7: Total Frames
 * Byte 8...: Chunk data
 * Byte cuối (4 bytes): CRC32
 */
static void Bootloader_Memory_Write(uint8_t *Host_Buffer)
{
	uint8_t Payload_Len = 0;
	uint8_t Status = FOTA_SUCCESS;
	uint16_t Frame_Index = 0;
	uint16_t Total_Frame = 0;

	Frame_Index = *((uint16_t*) (&Host_Buffer[4]));
	Total_Frame = *((uint16_t*) (&Host_Buffer[6]));
	uint8_t fw_number = Host_Buffer[2];

	if(fw_number != 1)
	{
		Status = FOTA_FAILED;
		Bootloader_Send_Data_To_Host((uint8_t*)&Status, 1);
		return;
	}
	if(!Frame_Index)
	{
		Address_to_write = FIRMWARE1_MEM_BASE;
		Temp_Firmware.address = Address_to_write;
		Temp_Firmware.crc = 0;
		Temp_Firmware.length = 0;
	}

	Payload_Len = Host_Buffer[3];	//Frame size

	/* Write the payload to the Flash memory */
	Status = Flash_Memory_Write_Payload((uint8_t*) &Host_Buffer[8], Address_to_write, Payload_Len);
	Status += Flash_Memory_Write_Payload((uint8_t*) &Host_Buffer[8], Address_to_write + FIRMWARE2_MEM_OFFSET, Payload_Len);
	Status += Flash_Memory_Write_Payload((uint8_t*) &Host_Buffer[8], Address_to_write + FIRMWARE3_MEM_OFFSET, Payload_Len);

	if (Status == FOTA_SUCCESS)
	{
		Address_to_write += Payload_Len;
		Temp_Firmware.length += Payload_Len;

		if (Frame_Index == (Total_Frame - 1))
		{
			Temp_Firmware.crc = Firmware_CRC_Verification(Temp_Firmware.address, Temp_Firmware.length);
			Temp_Firmware.fota_rqt = false;
			Flash_Write_Metadata(&Temp_Firmware);
		}
	}

	Bootloader_Send_Data_To_Host((uint8_t*) &Status, 1);
}

/**
 * Kiểm tra địa chỉ hợp lệ trong Flash
 * @param Jump_Address: Địa chỉ cần kiểm tra
 * @return ADDRESS_IS_VALID (1) nếu hợp lệ, ADDRESS_IS_INVALID (0) nếu không
 */
/*static uint8_t Host_Address_Verification(uint32_t Jump_Address)
{
	uint32_t physical_addr = Jump_Address;
    if ((physical_addr >= FLASH_BASE) && (physical_addr <= STM32H745_FLASH_END))
    {
        return ADDRESS_IS_VALID;
    }
    return ADDRESS_IS_INVALID;
}*/

/**
 * Kiểm tra CRC của dữ liệu
 * @param pData: Dữ liệu cần kiểm tra
 * @param Data_Len: Độ dài dữ liệu
 * @param Host_CRC: Giá trị CRC từ host
 * @return CRC_VERIFICATION_PASSED (1) nếu khớp, CRC_VERIFICATION_FAILED (0) nếu không
 */
static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC)
{
    uint8_t CRC_Status = FOTA_SUCCESS;
    uint32_t MCU_CRC_Calculated = 0;
    if (Data_Len == 0xFFFFFFFF)
        return MCU_CRC_Calculated;
    CRC->CR = CRC_CR_RESET;
    for (unsigned int i = 0; i < Data_Len; i++)
        CRC->DR = (uint32_t) pData[i];
    if (CRC->DR == Host_CRC)
    {
        CRC_Status = FOTA_SUCCESS;
    }
    else
    {
        CRC_Status = FOTA_FAILED;
    }

    return CRC_Status;
}

/**
 * Gửi NACK về host
 */
static void Bootloader_Send_NACK(void) {
    uint8_t Ack_Value = CBL_SEND_NACK;
    UART7_send_array((const char*) &Ack_Value, 1);
}

/**
 * Gửi dữ liệu về host
 * @param Host_Buffer: Dữ liệu cần gửi
 * @param Data_Len: Độ dài dữ liệu
 */
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len) {
    UART7_send_array((const char*) Host_Buffer, (uint8_t) Data_Len);
}


/**
 * Tính CRC cho firmware
 * @param start_address: Địa chỉ bắt đầu
 * @param length: Độ dài firmware
 * @return Giá trị CRC
 */
static uint32_t Firmware_CRC_Verification(uint32_t start_address, uint32_t length) {
    if (length == 0) return 0;

    uint32_t address = start_address;
    uint32_t end_address = start_address + length;
    uint32_t data = 0;

    CRC->CR = CRC_CR_RESET;

    while (address < end_address - 3) {
        data = *(uint32_t*)address;
        CRC->DR = data;
        address += 4;
    }

    while (address < end_address) {
        uint8_t byte = *(uint8_t*)address;
        CRC->DR = (uint32_t)byte;
        address++;
    }

    return CRC->DR;
}

/**
 * Thiết lập phiên bản firmware
 * @param Host_Buffer: Buffer chứa thông tin phiên bản
 */
static void Set_Firmware_Version(uint8_t *Host_Buffer)
{

    uint8_t status = FOTA_SUCCESS;
    if(Host_Buffer[2] != 1)		// fw_number
	{
    	status = FOTA_FAILED;
    	Bootloader_Send_Data_To_Host((uint8_t*)&status, 1);
    	return;
	}

    s_firmware_info fw_info;

    status = Flash_Read_Metadata(&fw_info);
    if(status != FOTA_SUCCESS)
    {
		Bootloader_Send_Data_To_Host((uint8_t*)&status, 1);
		return;
    }
    fw_info.version_major = Host_Buffer[3];
    fw_info.version_minor = Host_Buffer[4];
    fw_info.version_patch = Host_Buffer[5];

    status = Flash_Write_Metadata(&fw_info);
    Bootloader_Send_Data_To_Host((uint8_t*)&status, 1);
}

/**
 * Lấy phiên bản firmware
 * @param Host_Buffer: Buffer chứa yêu cầu
 */
static void Get_Firmware_Version(uint8_t *Host_Buffer)
{
    uint8_t status[6];
    memset(status, 0, sizeof(status));

    if(Host_Buffer[2] != 1)
	{
		status[0] = FOTA_FAILED;
		Bootloader_Send_Data_To_Host((uint8_t*)status, 6);
		return;
	}

	s_firmware_info fw_info;

	if(Flash_Read_Metadata(&fw_info) != FOTA_SUCCESS)
	{
		status[0] = FOTA_FAILED;
		Bootloader_Send_Data_To_Host((uint8_t*)status, 6);
		return;
	}

	uint16_t fw_size = ceil(fw_info.length / 1024.0f);
    status[0] = FOTA_SUCCESS;
    status[1] = fw_info.version_major;
    status[2] = fw_info.version_minor;
    status[3] = fw_info.version_patch;
    status[4] = (uint8_t)(fw_size >> 8);
    status[5] = (uint8_t)fw_size;
    Bootloader_Send_Data_To_Host((uint8_t*)status, 6);
}


/**
 * Ghi metadata của một firmware vào Flash.
 * @param fw: Con trỏ tới thông tin firmware (s_firmware_info).
 * @param fw_number: Số thứ tự firmware (1 cho bank 1, 2 cho bank 2).
 * @return FLASH_PAYLOAD_WRITE_PASSED nếu thành công, FLASH_PAYLOAD_WRITE_FAILED nếu lỗi hoặc đầu vào không hợp lệ.
 */
static uint8_t Flash_Write_Metadata(s_firmware_info* fw_info)
{
	if(fw_info == NULL) return FOTA_FAILED;

    uint32_t metadata_addr = METADATA_MEM_BASE;
    uint32_t fw_info_size = sizeof(s_firmware_info);
    /* Chuẩn bị mảng tạm */
    uint8_t temp_buffer[64] __attribute__((aligned(4))); /* Đủ lớn cho metadata, căn chỉnh 4 byte */
    memset(temp_buffer, 0xFF, sizeof(temp_buffer));
    memcpy(temp_buffer, fw_info, fw_info_size);

    HAL_FLASH_Unlock();

	FLASH_EraseInitTypeDef EraseInitStruct = {0};
	uint32_t SectorError = 0;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	//EraseInitStruct.Banks = FLASH_BANK_1;
	EraseInitStruct.Sector = 3; 						// Metadata ở Sector 3
	EraseInitStruct.NbSectors = 1;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
	{
		HAL_FLASH_Lock();
		return FOTA_FAILED;
	}

    for (uint32_t i = 0; i < fw_info_size; i += 4)
    {
        uint32_t temp_data = *(uint32_t *)(temp_buffer + i);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, metadata_addr + i, temp_data) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return FOTA_FAILED;
        }
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
static uint8_t Flash_Read_Metadata(s_firmware_info* fw_info)
{
	uint32_t metadata_addr = METADATA_MEM_BASE;

    uint8_t data[32];
    uint32_t fw_info_size = sizeof(s_firmware_info);

    memcpy(data, (void*)metadata_addr, sizeof(data));
    memcpy(fw_info, data, fw_info_size);

    if((fw_info->address == 0xffffffff)||
	(fw_info->length == 0xffffffff)||
	(fw_info->crc == 0xffffffff))
    	return FOTA_FAILED;
    return FOTA_SUCCESS;
}



/**
 * Kiểm tra firmware có sẵn
 */
static uint8_t Firmware_Check_Available(void)
{
    uint32_t CRC_Result = 0;
    s_firmware_info fw_info;

    // Kiểm tra Firmware 1
    if(Flash_Read_Metadata(&fw_info) == FOTA_FAILED)	return FOTA_FAILED;

    if (fw_info.length > 0)
    {
		CRC_Result = Firmware_CRC_Verification(fw_info.address, fw_info.length);
		if (CRC_Result != fw_info.crc)
		{
			if (Flash_CopyData(FIRMWARE2_MEM_BASE, FIRMWARE1_MEM_BASE, fw_info.length) != FOTA_SUCCESS)		//copy fw2 to fw1 base
			{
				if (Flash_CopyData(FIRMWARE3_MEM_BASE, FIRMWARE1_MEM_BASE, fw_info.length) != FOTA_SUCCESS)		//copy fw3 to fw1 base
				{
					return FOTA_FAILED;
				}
			}
			CRC_Result = Firmware_CRC_Verification(fw_info.address, fw_info.length);
		}

		if (CRC_Result == fw_info.crc)
		{
			uint32_t app_address = FIRMWARE1_MEM_BASE;
			fw_info.fota_rqt = false;
			if(Flash_Write_Metadata(&fw_info) != FOTA_SUCCESS) return FOTA_FAILED;
			if(Jump_To_App(app_address) != FOTA_SUCCESS) return FOTA_FAILED;
		}
    }
    return FOTA_FAILED;
}

static uint8_t Flash_CopyData(uint32_t src_addr, uint32_t dst_addr, uint32_t length)
{
    if (length == 0)  return FOTA_FAILED;
	// First clear flash fw1
	if (Perform_Flash_Erase(FIRMWARE1_SECTOR, FIRMWARE1_NUM_SECTORS) == FOTA_FAILED)	return FOTA_FAILED;

	HAL_FLASH_Unlock();

    uint32_t i = 0;
    while (i < length)
    {
    	uint8_t temp_buffer[4] __attribute__((aligned(4)));
		memset(temp_buffer, 0xFF, 4);
        uint16_t bytes_to_write = (length - i > 4) ? 4 : (length - i);
        memcpy(temp_buffer, (void *)(src_addr + i), bytes_to_write);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dst_addr + i, *(uint32_t *)temp_buffer) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return FOTA_FAILED;
        }
        i += 4; // Tăng bước nhảy 4 byte
    }

	HAL_FLASH_Lock();
	return FOTA_SUCCESS;
}





