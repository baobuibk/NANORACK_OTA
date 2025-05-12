/************************************             Includes               ************************************/
#include "bootloader.h"


extern usart_meta_t USART1_meta;
extern usart_meta_t *p_USART1_meta;

typedef struct _s_firmware_info_ {
    bool is_Available;
    uint32_t address;      // Địa chỉ firmware
    uint32_t length;       // Độ dài firmware
    uint32_t crc;          // CRC cho firmware
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
} s_firmware_info;

s_firmware_info Temp_Firmware;
static uint32_t Address_to_write;
volatile uint32_t boot_timeout = 0;
bool check_connection = false;
/************************************    Static Functions Declarations  ************************************/
static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer);
//static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer);
static void Bootloader_Jump_To_User_App(void);
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer);
static void Bootloader_Memory_Write(uint8_t *Host_Buffer);
//static void RESET_CHIP(void);
void Bootloader_CRC_Verify_Seq(uint8_t *pData, uint32_t Data_Len, uint32_t *InitVal);
static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC);
static void Bootloader_Send_NACK(void);
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len);
//static uint8_t Host_Address_Verification(uint32_t Jump_Address);
static uint8_t Perform_Flash_Erase(uint8_t Sector_Number, uint8_t Number_Of_Sectors);
static void Set_Firmware_Version(uint8_t *Host_Buffer);
static void Get_Firmware_Version(uint8_t *Host_Buffer);
static uint32_t Firmware_CRC_Verification(uint32_t start_address, uint32_t length);
static uint8_t Flash_Write_Metadata(s_firmware_info* fw1, uint8_t fw_select);
static uint8_t Flash_Read_Metadata(s_firmware_info* fw_info, uint8_t fw_number);
static void Bootloader_check_connection(uint8_t *Host_Buffer);
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
    while (!rbuffer_empty(&p_USART1_meta->rb_rx))
    {
        data = rbuffer_remove(&p_USART1_meta->rb_rx);
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
                if ((Bootloader_CRC_Verify((uint8_t*) &BL_Host_Buffer[0], Host_CMD_Packet_Len - 4, Host_CRC32) == CRC_VERIFICATION_FAILED)&&
					(BL_Host_Buffer[1] != CBL_CHECK_CONNECTION))
                {
                    Bootloader_Send_NACK();
                }
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
						Bootloader_check_connection(BL_Host_Buffer);
						break;
					default:
						break;
                }
            }
        }
    }
}

/************************************    Static Functions Implementations  ************************************/

/**
 * Lấy số nhận dạng chip và gửi về host
 * @param Host_Buffer: Buffer từ host (không dùng trong hàm này)
 */
static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer) {
    uint16_t MCU_Identification_Number = 0;

    // Lấy Device ID từ DBGMCU->IDCODE
    MCU_Identification_Number = (uint16_t)(DBGMCU->IDCODE & 0x00000FFF);

    // Gửi về host
    Bootloader_Send_Data_To_Host((uint8_t*)&MCU_Identification_Number, 2);
}

/**
 * Đọc mức bảo vệ RDP trên STM32F746 và gửi về host
 * @param Host_Buffer: Buffer từ host (không dùng trong hàm này)
 */

//static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer)
//{
//    uint8_t RDP_Level = 0;
//
//    // Đọc mức RDP từ FLASH->OPTR
//
//    FLASH_OBProgramInitTypeDef FLASH_OBProgram;
//    	/* Get the Option byte configuration */
//	HAL_FLASHEx_OBGetConfig(&FLASH_OBProgram);
//
//    RDP_Level = (uint8_t) (FLASH_OBProgram.RDPLevel);
//
//    // Ánh xạ giá trị RDP sang mức đơn giản
//    if (RDP_Level == 0xAA) {
//        RDP_Level = CBL_ROP_LEVEL_0;  // Level 0
//    } else if (RDP_Level == 0xCC) {
//        RDP_Level = CBL_ROP_LEVEL_2;  // Level 2
//    } else {
//        RDP_Level = CBL_ROP_LEVEL_1;  // Level 1
//    }
//
//    // Gửi về host
//    Bootloader_Send_Data_To_Host((uint8_t*)&RDP_Level, 1);
//}

/**
 * Nhảy đến ứng dụng người dùng
 * @param app_address: Địa chỉ bắt đầu của ứng dụng
 * @return true nếu nhảy thành công, false nếu thất bại
 */
bool Jump_To_App(uint32_t app_address) {
    if (*((volatile uint32_t*)app_address) != 0xFFFFFFFF)
    {
        uint8_t appExists = 1;

//		HAL_HSEM_Release(HSEM_ID_0,0);

        Bootloader_Send_Data_To_Host((uint8_t*)&appExists, 1);
        HAL_Delay(1);

        while (!rbuffer_empty(&p_USART1_meta->rb_tx));
        __disable_irq();

        for (uint8_t i = 0; i < 8; i++)
        {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }

        LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_USART1);
        LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_USART1);

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

        return true;
    }
    return false;
}

/**
 * Xử lý lệnh nhảy đến ứng dụng từ host
 * @param Host_Buffer: Buffer chứa địa chỉ ứng dụng
 */
static void Bootloader_Jump_To_User_App(void)
{
	uint32_t app_address = FIRMWARE_BANK1_BASE;
	if (!Jump_To_App(app_address))
	{
		uint8_t appExists = ADDRESS_IS_INVALID;
		Bootloader_Send_Data_To_Host(&appExists, 1);
	}
}

/**
 * Thực hiện xóa Flash: xóa từng sector hoặc mass erase
 * @param Sector_Number: Số sector bắt đầu (0-15) hoặc CBL_FLASH_MASS_ERASE
 * @param Number_Of_Sectors: Số sector cần xóa
 * @return SUCCESSFUL_ERASE (0), UNSUCCESSFUL_ERASE (1), hoặc INVALID_SECTOR_NUMBER (2)
 */
static uint8_t Perform_Flash_Erase(uint8_t Sector_Number, uint8_t Number_Of_Sectors)
{
    FLASH_EraseInitTypeDef EraseInitStruct = {0};
    uint32_t SectorError = 0;

    // Kiểm tra số sector hợp lệ
    if (Number_Of_Sectors > CBL_FLASH_MAX_SECTOR_NUMBER)
    {
        return INVALID_SECTOR_NUMBER;
    }
    // Kiểm tra Sector_Number hợp lệ
    if ((Sector_Number != CBL_FLASH_MASS_ERASE) && (Sector_Number > 16))
    {
        return UNSUCCESSFUL_ERASE;
    }

    HAL_FLASH_Unlock(); // Mở khóa flash
    // Xử lý mass erase
    if (Sector_Number == CBL_FLASH_MASS_ERASE)
    {
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_MASSERASE;
        EraseInitStruct.Banks = FLASH_BANK_BOTH; // Xóa cả hai bank
        EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return UNSUCCESSFUL_ERASE;
        }
        HAL_FLASH_Lock();
        return SUCCESSFUL_ERASE;
    }
    // Xóa từng sector
    if (Sector_Number < 8)
    {
    	EraseInitStruct.Banks = FLASH_BANK_1;
    	EraseInitStruct.Sector = Sector_Number;
    }
    else
    {
    	EraseInitStruct.Banks = FLASH_BANK_2;
    	EraseInitStruct.Sector = Sector_Number - 8;
    }
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.NbSectors = Number_Of_Sectors;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return UNSUCCESSFUL_ERASE;
    }

    HAL_FLASH_Lock();
    return SUCCESSFUL_ERASE;
}

/**
 * Xóa firmware trong Flash dựa trên lựa chọn từ host
 * @param Host_Buffer: Dữ liệu từ host, Host_Buffer[2] chọn firmware (1: Bank 1, 2: Bank 2)
 */
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer)
{
    uint8_t Erase_Status = 0;
    s_firmware_info fw_info;
	fw_info.is_Available = false;
	fw_info.address = 0;
	fw_info.length = 0;
	fw_info.crc = 0;
	fw_info.version_major = 0;
	fw_info.version_minor = 0;
	fw_info.version_patch = 0;

    switch (Host_Buffer[2]) {
        case 1: // Xóa Firmware 1 (Bank 2, 4 sector)
            Erase_Status = Perform_Flash_Erase(FIRMWARE1_SECTOR, FIRMWARE_NUM_SECTORS);
            Erase_Status += Flash_Write_Metadata(&fw_info, 1);
		break;

        case 2: // Xóa Firmware 2 (Bank 2, 4 sector)
            Erase_Status = Perform_Flash_Erase(FIRMWARE2_SECTOR, FIRMWARE_NUM_SECTORS);
            Erase_Status += Flash_Write_Metadata(&fw_info, 2);
		break;

        default:
            Erase_Status = UNSUCCESSFUL_ERASE; // Lựa chọn không hợp lệ
		break;
    }

    /* Gửi kết quả về host */
    if (Erase_Status > 1)
    {
    	Erase_Status = SUCCESSFUL_ERASE;
    	Bootloader_Send_Data_To_Host((uint8_t*)&Erase_Status, 1);
    	return;
    }
    Erase_Status = UNSUCCESSFUL_ERASE;
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
	if (Host_Payload == NULL || Payload_Len == 0)
	{
		return FLASH_PAYLOAD_WRITE_FAILED;
	}
	// Mở khóa flash
	if (HAL_FLASH_Unlock() != HAL_OK)
	{
		return FLASH_PAYLOAD_WRITE_FAILED;
	}

    uint16_t i = 0;
    uint8_t buffer[32] __attribute__((aligned(32))); 	// Bộ đệm 32 byte cho 256-bit

    while (i < Payload_Len)
    {
        memset(buffer, 0xFF, sizeof(buffer)); 			// Padding mặc định là 0xFF
        // Tính số byte cần ghi (tối đa 32 byte)
        uint16_t bytes_to_write = (Payload_Len - i > 32) ? 32 : (Payload_Len - i);
        memcpy(buffer, &Host_Payload[i], bytes_to_write);
        // Ghi 32-bit vào flash
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, Payload_Start_Address + i, (uint32_t)buffer) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return FLASH_PAYLOAD_WRITE_FAILED;
        }
        i += 32; // Tăng bước nhảy 4 byte
    }
    HAL_FLASH_Lock();
    return FLASH_PAYLOAD_WRITE_PASSED;
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
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	uint16_t Frame_Index = 0;
	uint16_t Total_Frame = 0;

	Frame_Index = *((uint16_t*) (&Host_Buffer[4]));
	Total_Frame = *((uint16_t*) (&Host_Buffer[6]));

	uint8_t fw_number = Host_Buffer[2];
	if((fw_number > 2)||(fw_number < 1))
	{
		Address_Verification = ADDRESS_IS_INVALID;
		Bootloader_Send_Data_To_Host((uint8_t*) &Address_Verification, 1);
		return;
	}
	if(!Frame_Index)
	{
		Address_to_write = (fw_number == 1) ? FIRMWARE_BANK1_BASE : FIRMWARE_BANK2_BASE;
		Temp_Firmware.address = Address_to_write;
		Temp_Firmware.crc = 0;
		Temp_Firmware.length = 0;
	}

	Payload_Len = Host_Buffer[3];	//Frame size

	/* Write the payload to the Flash memory */
	Flash_Payload_Write_Status = Flash_Memory_Write_Payload((uint8_t*) &Host_Buffer[8], Address_to_write, Payload_Len);
	if (Flash_Payload_Write_Status == FLASH_PAYLOAD_WRITE_PASSED)
	{
		Address_to_write += Payload_Len;
		Temp_Firmware.length += Payload_Len;

		if (Frame_Index == (Total_Frame - 1))
		{
			Temp_Firmware.crc = Firmware_CRC_Verification(Temp_Firmware.address, Temp_Firmware.length);
			Temp_Firmware.is_Available = true;
			Flash_Write_Metadata(&Temp_Firmware, fw_number);
		}
		Bootloader_Send_Data_To_Host((uint8_t*) &Flash_Payload_Write_Status, 1);
	}

	else Bootloader_Send_Data_To_Host((uint8_t*) &Flash_Payload_Write_Status, 1);

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
    uint8_t CRC_Status = CRC_VERIFICATION_FAILED;
    uint32_t MCU_CRC_Calculated = 0;
    if (Data_Len == 0xFFFFFFFF)
        return MCU_CRC_Calculated;
    CRC->CR = CRC_CR_RESET;
    for (unsigned int i = 0; i < Data_Len; i++)
        CRC->DR = (uint32_t) pData[i];
    if (CRC->DR == Host_CRC)
    {
        CRC_Status = CRC_VERIFICATION_PASSED;
    }
    else
    {
        CRC_Status = CRC_VERIFICATION_FAILED;
    }

    return CRC_Status;
}

/**
 * Tính CRC tuần tự cho dữ liệu
 * @param pData: Dữ liệu cần tính
 * @param Data_Len: Độ dài dữ liệu
 * @param InitVal: Giá trị CRC tính được
 */
void Bootloader_CRC_Verify_Seq(uint8_t *pData, uint32_t Data_Len, uint32_t *InitVal)
{
    CRC->CR = CRC_CR_RESET;
    for (unsigned int i = 0; i < Data_Len; i++)
    {
        CRC->DR = (uint32_t) pData[i];
    }
    *InitVal = CRC->DR;
}

/**
 * Gửi NACK về host
 */
static void Bootloader_Send_NACK(void) {
    uint8_t Ack_Value = CBL_SEND_NACK;
    USART1_send_array((const char*) &Ack_Value, 1);
}

/**
 * Gửi dữ liệu về host
 * @param Host_Buffer: Dữ liệu cần gửi
 * @param Data_Len: Độ dài dữ liệu
 */
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len) {
    USART1_send_array((const char*) Host_Buffer, (uint8_t) Data_Len);
}

/**
 * Reset chip
 */
/*static void RESET_CHIP(void) {
    uint8_t appExists = 1;
    Bootloader_Send_Data_To_Host((uint8_t*)&appExists, 1);
    HAL_Delay(1);

    while (!rbuffer_empty(&p_USART1_meta->rb_tx));

    __disable_irq();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;

    NVIC_SystemReset();
}*/

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

    uint8_t status = FLASH_PAYLOAD_WRITE_FAILED;
    uint8_t fw_number = Host_Buffer[2];
    if((fw_number < 1)||(fw_number > 2))
	{
    	status = FLASH_PAYLOAD_WRITE_FAILED;
    	Bootloader_Send_Data_To_Host((uint8_t*)&status, 1);
    	return;
	}

    s_firmware_info fw_info;

    if(!Flash_Read_Metadata(&fw_info, fw_number))
    {
    	status = FLASH_PAYLOAD_WRITE_FAILED;
		Bootloader_Send_Data_To_Host((uint8_t*)&status, 1);
		return;
    }

    fw_info.version_major = Host_Buffer[3];
    fw_info.version_minor = Host_Buffer[4];
    fw_info.version_patch = Host_Buffer[5];

    status = Flash_Write_Metadata(&fw_info, fw_number);
    Bootloader_Send_Data_To_Host((uint8_t*)&status, 1);
}

/**
 * Lấy phiên bản firmware
 * @param Host_Buffer: Buffer chứa yêu cầu
 */
static void Get_Firmware_Version(uint8_t *Host_Buffer)
{
    uint8_t fw_number = Host_Buffer[2];
    uint8_t status[7];
    memset(status, 0, sizeof(status));

    if((fw_number < 1)||(fw_number > 2))
	{
		status[0] = FLASH_PAYLOAD_WRITE_FAILED;
		Bootloader_Send_Data_To_Host((uint8_t*)status, 7);
		return;
	}

	s_firmware_info fw_info;

	if(!Flash_Read_Metadata(&fw_info, fw_number))
	{
		status[0] = FLASH_PAYLOAD_WRITE_FAILED;
		Bootloader_Send_Data_To_Host((uint8_t*)status, 7);
		return;
	}
	uint16_t fw_size = ceil(fw_info.length / 1024.0f);
    status[0] = FLASH_PAYLOAD_WRITE_PASSED;
    status[1] = fw_info.version_major;
    status[2] = fw_info.version_minor;
    status[3] = fw_info.version_patch;
    status[4] = fw_info.is_Available;
    status[5] = (uint8_t)(fw_size >> 8);
    status[6] = (uint8_t)fw_size;
    Bootloader_Send_Data_To_Host((uint8_t*)status, 7);
}


/**
 * Ghi metadata của một firmware vào Flash.
 * @param fw: Con trỏ tới thông tin firmware (s_firmware_info).
 * @param fw_number: Số thứ tự firmware (1 cho bank 1, 2 cho bank 2).
 * @return FLASH_PAYLOAD_WRITE_PASSED nếu thành công, FLASH_PAYLOAD_WRITE_FAILED nếu lỗi hoặc đầu vào không hợp lệ.
 */
static uint8_t Flash_Write_Metadata(s_firmware_info* fw, uint8_t fw_number)
{
	if((fw == NULL)||(fw_number < 1)||(fw_number > 2))
	{
		return FLASH_PAYLOAD_WRITE_FAILED;
	}
	uint32_t metadata_addr = (fw_number == 1) ? METADATA_BANK1_BASE : METADATA_BANK2_BASE;
    uint8_t bank = (fw_number == 1) ? FLASH_BANK_1 : FLASH_BANK_2;

    uint8_t data[32] __attribute__((aligned(32)));
    uint32_t fw_info_size = sizeof(s_firmware_info);
    memset(data, 0xFF, sizeof(data));
    memcpy(data, fw, fw_info_size);

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
		return FLASH_PAYLOAD_WRITE_FAILED;
	}

	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, metadata_addr, (uint32_t)data) != HAL_OK)
	{
		HAL_FLASH_Lock();
		return FLASH_PAYLOAD_WRITE_FAILED;
	}

    HAL_FLASH_Lock();
    return FLASH_PAYLOAD_WRITE_PASSED;
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
	if((fw_number < 1)||(fw_number > 2))
	{
		return FLASH_PAYLOAD_WRITE_FAILED;
	}

	uint32_t metadata_addr = (fw_number == 1) ? METADATA_BANK1_BASE : METADATA_BANK2_BASE;

    uint8_t data[32] __attribute__((aligned(32)));
    uint32_t fw_info_size = sizeof(s_firmware_info);

    memcpy(data, (void*)metadata_addr, sizeof(data));
    memcpy(fw_info, data, fw_info_size);

    if((fw_info->address == 0xffffffff)||
	(fw_info->length == 0xffffffff)||
	(fw_info->crc == 0xffffffff))
    	return FLASH_PAYLOAD_WRITE_FAILED;
    return FLASH_PAYLOAD_WRITE_PASSED;
}



static void Bootloader_check_connection(uint8_t *Host_Buffer)
{
	if((Host_Buffer[2] == 't')||(Host_Buffer[3] == 'm')||
	(Host_Buffer[4] == 'o')||(Host_Buffer[5] == 'd')||(Host_Buffer[6] == 'e'))
	{
		check_connection = true;
		uint8_t status[3] = {0x01, 'O', 'K'};
	    Bootloader_Send_Data_To_Host((uint8_t*)status, 3);
	}
}


/**
 * Kiểm tra firmware có sẵn
 */
static uint8_t Firmware_Check_Available(void)
{
	uint8_t status = 0;
    uint32_t CRC_Result = 0;
    s_firmware_info fw_info;

    // Kiểm tra Firmware 1
    Flash_Read_Metadata(&fw_info, 1);

    if (fw_info.is_Available && (fw_info.length > 0))
    {
		CRC_Result = Firmware_CRC_Verification(fw_info.address, fw_info.length);
		if (CRC_Result == fw_info.crc)
		{
			status ++;
		}
    }

    // Kiểm tra Firmware 2
    Flash_Read_Metadata(&fw_info, 2);

    if (fw_info.is_Available && (fw_info.length > 0))
	{
		CRC_Result = Firmware_CRC_Verification(fw_info.address, fw_info.length);
		if (CRC_Result == fw_info.crc)
		{
			status++;
		}
	}


    if(status > 1)			// Cả 2 core đều có sẵn firmware
    {
    	uint32_t app_address = FIRMWARE_BANK1_BASE;
    	Jump_To_App(app_address);
    }
    return 0;
}


/**
 * Kiểm tra firmware có sẵn và nhảy đến nếu hợp lệ
 */
void Bootloader_Check_Available(void*)
{
	if(check_connection) boot_timeout = 0;
	if(boot_timeout > BL_TIMEOUT)
	{
		if(!Firmware_Check_Available())		//fail
		{
			check_connection = true;
		}
	}
}
