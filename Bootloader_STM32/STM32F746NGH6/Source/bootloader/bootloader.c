/************************************             Includes               ************************************/
#include "bootloader.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "UART.h"
#include "flash.h"
#include <stm32f7xx_hal.h>
#include <stm32f7xx_ll_bus.h>

extern usart_meta_t USART6_meta;
extern usart_meta_t *p_USART6_meta;

typedef struct _s_firmware_info_ {
    bool is_Available;
    uint32_t address;      // Địa chỉ firmware
    uint32_t length;       // Độ dài firmware
    uint32_t crc;          // CRC cho firmware
    uint8_t bank;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
} s_firmware_info;

s_firmware_info Firmware1 = {
    .is_Available = false,
    .address = FIRMWARE_BANK1_BASE,
    .length = 0,
    .crc = 0,
    .bank = 1,
    .version_major = 1,
    .version_minor = 0,
    .version_patch = 0,
};

s_firmware_info Firmware2 = {
    .is_Available = false,
    .address = FIRMWARE_BANK2_BASE,
    .length = 0,
    .crc = 0,
    .bank = 2,
    .version_major = 2,
    .version_minor = 0,
    .version_patch = 0,
};

uint8_t Firmware_Select = 0xff;
s_firmware_info Temp_Firmware;

/************************************    Static Functions Declarations  ************************************/
static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer);
static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer);
static void Bootloader_Jump_To_User_App(uint8_t *Host_Buffer);
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer);
static void Bootloader_Memory_Write(uint8_t *Host_Buffer);
static void RESET_CHIP(void);
void Bootloader_CRC_Verify_Seq(uint8_t *pData, uint32_t Data_Len, uint32_t *InitVal);
static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC);
static void Bootloader_Send_NACK(void);
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len);
static uint8_t Host_Address_Verification(uint32_t Jump_Address);
static uint8_t Perform_Flash_Erase(uint8_t Sector_Number, uint8_t Number_Of_Sectors);
static void Set_Firmware_Version(uint8_t *Host_Buffer);
static void Get_Firmware_Version(uint8_t *Host_Buffer);
static uint32_t Firmware_CRC_Verification(uint32_t start_address, uint32_t length);
static FLASH_StatusTypeDef Flash_Write_All_Metadata(s_firmware_info* fw1, s_firmware_info* fw2, uint8_t fw_select);
static FLASH_StatusTypeDef Flash_Read_All_Metadata(s_firmware_info* fw1, s_firmware_info* fw2, uint8_t* fw_select, uint8_t bank_select);

/************************************    Global Variables Definitions     ************************************/
static uint8_t BL_Host_Buffer[150];
static uint8_t appExists = 0;
uint8_t frame_index = 0;
uint8_t frame_length = 0;
bool receiving_frame = false;
uint16_t frame_timeout = 0;

/************************************ Software Interfaces Implementations ************************************/
void BL_UART_Fetch_Host_Command(void) {
    uint8_t data;
    if (receiving_frame) {
        if (frame_timeout++ > 1000) {
            receiving_frame = false;
            frame_length = 0;
            frame_index = 0;
            frame_timeout = 0;
        }
    }
    while (!rbuffer_empty(&p_USART6_meta->rb_rx)) {
        data = rbuffer_remove(&p_USART6_meta->rb_rx);
        if (!receiving_frame) {
            // Nhận byte đầu tiên (FRAME LENGTH)
            frame_timeout = 0;
            frame_length = data;
            if (frame_length > 0 && frame_length < 255) {
                BL_Host_Buffer[0] = frame_length;
                frame_index = 1;
                receiving_frame = true;
            } else {
                // Nếu frame_length không hợp lệ, reset trạng thái
                frame_index = 0;
                receiving_frame = false;
            }
        } else {
            BL_Host_Buffer[frame_index++] = data;

            if (frame_index >= frame_length + 1) {
                receiving_frame = false;
                frame_length = 0;
                uint16_t Host_CMD_Packet_Len = 0;
                uint32_t Host_CRC32 = 0;
                /* Extract the CRC32 and packet length sent by the HOST */
                Host_CMD_Packet_Len = BL_Host_Buffer[0] + 1;
                Host_CRC32 = *((uint32_t*) ((BL_Host_Buffer
                        + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
                if (CRC_VERIFICATION_FAILED
                        == Bootloader_CRC_Verify((uint8_t*) &BL_Host_Buffer[0],
                                Host_CMD_Packet_Len - 4, Host_CRC32)) {
                    Bootloader_Send_NACK();
                }
                switch (BL_Host_Buffer[1]) {
                case CBL_GET_CID_CMD:
                    Bootloader_Get_Chip_Identification_Number(BL_Host_Buffer);
                    break;
                case CBL_GET_RDP_STATUS_CMD:
                    Bootloader_Read_Protection_Level(BL_Host_Buffer);
                    break;
                case CBL_GO_TO_ADDR_CMD:
                    Bootloader_Jump_To_User_App(BL_Host_Buffer);
                    break;
                case CBL_FLASH_ERASE_CMD:
                    Bootloader_Erase_Flash(BL_Host_Buffer);
                    break;
                case CBL_MEM_WRITE_CMD:
                    Bootloader_Memory_Write(BL_Host_Buffer);
                    break;
                case CBL_RESET_CHIP:
                    RESET_CHIP();
                    break;
                case CBL_SET_VERSION:
                    Set_Firmware_Version(BL_Host_Buffer);
                    break;
                case CBL_GET_VERSION:
                    Get_Firmware_Version(BL_Host_Buffer);
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
static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer) {
    uint8_t RDP_Level = 0;

    // Đọc mức RDP từ FLASH->OPTR

    FLASH_OBProgramInitTypeDef FLASH_OBProgram;
    	/* Get the Option byte configuration */
	HAL_FLASHEx_OBGetConfig(&FLASH_OBProgram);

    RDP_Level = (uint8_t) (FLASH_OBProgram.RDPLevel);

    // Ánh xạ giá trị RDP sang mức đơn giản
    if (RDP_Level == 0xAA) {
        RDP_Level = CBL_ROP_LEVEL_0;  // Level 0
    } else if (RDP_Level == 0xCC) {
        RDP_Level = CBL_ROP_LEVEL_2;  // Level 2
    } else {
        RDP_Level = CBL_ROP_LEVEL_1;  // Level 1
    }

    // Gửi về host
    Bootloader_Send_Data_To_Host((uint8_t*)&RDP_Level, 1);
}

/**
 * Nhảy đến ứng dụng người dùng
 * @param app_address: Địa chỉ bắt đầu của ứng dụng
 * @return true nếu nhảy thành công, false nếu thất bại
 */
_Bool Jump_To_App(uint32_t app_address) {
    if (*((volatile uint32_t*)app_address) != 0xFFFFFFFF) {
        uint8_t appExists = 1;
        Bootloader_Send_Data_To_Host((uint8_t*)&appExists, 1);
        HAL_Delay(1);

        while (!rbuffer_empty(&p_USART6_meta->rb_tx));

        __disable_irq();

        for (uint8_t i = 0; i < 8; i++) {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }

        LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_USART6);
        LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_USART6);

        if (SCB->CCR & SCB_CCR_DC_Msk) {
            SCB_CleanInvalidateDCache();
            SCB_DisableDCache();
        }
        if (SCB->CCR & SCB_CCR_IC_Msk) {
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
static void Bootloader_Jump_To_User_App(uint8_t *Host_Buffer) {
    if (Host_Buffer == NULL) return;

    uint32_t app_address = ((uint32_t)Host_Buffer[2] << 24) |
                           ((uint32_t)Host_Buffer[3] << 16) |
                           ((uint32_t)Host_Buffer[4] << 8)  |
                           ((uint32_t)Host_Buffer[5]);

    if (app_address == FIRMWARE_BANK1_BASE || app_address == FIRMWARE_BANK2_BASE) {
        if (!Jump_To_App(app_address)) {
            uint8_t appExists = ADDRESS_IS_INVALID;
            Bootloader_Send_Data_To_Host(&appExists, 1);
        }
    } else {
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
static uint8_t Perform_Flash_Erase(uint8_t Sector_Number, uint8_t Number_Of_Sectors) {
    FLASH_EraseInitTypeDef EraseInitStruct = {0};
    uint32_t SectorError = 0;

    // Kiểm tra số sector hợp lệ
    if (Number_Of_Sectors > CBL_FLASH_MAX_SECTOR_NUMBER) {
        return INVALID_SECTOR_NUMBER;
    }

    // Kiểm tra Sector_Number hợp lệ
    if (Sector_Number > (CBL_FLASH_MAX_SECTOR_NUMBER - 1) && Sector_Number != CBL_FLASH_MASS_ERASE) {
        return UNSUCCESSFUL_ERASE;
    }

    HAL_FLASH_Unlock(); // Mở khóa flash

    // Xử lý mass erase
    if (Sector_Number == CBL_FLASH_MASS_ERASE) {
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_MASSERASE;
        EraseInitStruct.Banks = FLASH_BANK_BOTH; // Xóa cả hai bank
        EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
            HAL_FLASH_Lock();
            return UNSUCCESSFUL_ERASE;
        }
        HAL_FLASH_Lock();
        return SUCCESSFUL_ERASE;
    }

    // Xóa từng sector
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    if (Sector_Number < 8) {
        EraseInitStruct.Banks = FLASH_BANK_1;
        EraseInitStruct.Sector = Sector_Number;
    } else {
        EraseInitStruct.Banks = FLASH_BANK_2;
        EraseInitStruct.Sector = Sector_Number - 8;
    }
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.NbSectors = Number_Of_Sectors;



    // Kiểm tra swap bank
//    if (FLASH->OPTCR & FLASH_OPTCR_SWAP_BANK) {
//        EraseInitStruct.Banks = (EraseInitStruct.Banks == FLASH_BANK_1) ? FLASH_BANK_2 : FLASH_BANK_1;
//    }

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
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
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer) {
    uint8_t Erase_Status = 0;

    /* Perform erase of the selected firmware */
    switch (Host_Buffer[2]) {
        case 1: // Xóa Firmware 1 (Bank 1, Sector 2-5)
            Erase_Status = Perform_Flash_Erase(FIRMWARE1_SECTOR, FIRMWARE_NUM_SECTORS);
            if (Erase_Status == SUCCESSFUL_ERASE) {
                Firmware1.is_Available = false;
                Firmware1.address = FIRMWARE_BANK1_BASE;
                Firmware1.length = 0;
                Firmware1.crc = 0;
                //Flash_Write_All_Metadata(&Firmware1, &Firmware2, Firmware_Select);
            }
            break;

        case 2: // Xóa Firmware 2 (Bank 2, Sector 2-5)
            Erase_Status = Perform_Flash_Erase(FIRMWARE2_SECTOR, FIRMWARE_NUM_SECTORS);
            if (Erase_Status == SUCCESSFUL_ERASE) {
                Firmware2.is_Available = false;
                Firmware2.address = FIRMWARE_BANK2_BASE;
                Firmware2.length = 0;
                Firmware2.crc = 0;
                //Flash_Write_All_Metadata(&Firmware1, &Firmware2, Firmware_Select);
            }
            break;

        default:
            Erase_Status = INVALID_SECTOR_NUMBER; // Lựa chọn không hợp lệ
            break;
    }

    /* Gửi kết quả về host */
    Bootloader_Send_Data_To_Host((uint8_t*)&Erase_Status, 1);
}

/**
 * Ghi payload vào Flash với độ dài bất kỳ, tự động chèn 0xFF nếu cần
 * @param Host_Payload: Dữ liệu cần ghi
 * @param Payload_Start_Address: Địa chỉ bắt đầu trong Flash
 * @param Payload_Len: Độ dài dữ liệu (byte)
 * @return FLASH_PAYLOAD_WRITE_PASSED (0) nếu thành công, FLASH_PAYLOAD_WRITE_FAILED (1) nếu lỗi
 */
static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_Payload, uint32_t Payload_Start_Address, uint16_t Payload_Len) {
	HAL_FLASH_Unlock(); // Mở khóa flash

    uint16_t i = 0;
    while (i < Payload_Len) {
        uint8_t buffer[4] __attribute__((aligned(4))); // Bộ đệm 8 byte cho 64-bit
        memset(buffer, 0xFF, sizeof(buffer)); // Padding mặc định là 0xFF

        // Tính số byte cần ghi (tối đa 8 byte)
        uint16_t bytes_to_write = (Payload_Len - i > 4) ? 4 : (Payload_Len - i);
        memcpy(buffer, &Host_Payload[i], bytes_to_write);

        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR);
        // Ghi 64-bit vào flash
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Payload_Start_Address + i, *(uint32_t *)buffer) != HAL_OK) {
            HAL_FLASH_Lock();
            return FLASH_PAYLOAD_WRITE_FAILED;
        }

        i += 4; // Tăng bước nhảy 8 byte
    }

    HAL_FLASH_Lock();
    return FLASH_PAYLOAD_WRITE_PASSED;
}

/**
 * Ghi firmware vào Flash dựa trên lệnh từ host
 * @param Host_Buffer: Buffer chứa dữ liệu firmware
 */
static void Bootloader_Memory_Write(uint8_t *Host_Buffer) {

	uint32_t HOST_Address = 0;
	uint8_t Payload_Len = 0;
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	uint16_t Frame_Index = 0;
	uint16_t Total_Frame = 0;

	Frame_Index = *((uint16_t*) (&Host_Buffer[7]));
	Total_Frame = *((uint16_t*) (&Host_Buffer[9]));

	/* Extract the CRC32 and packet length sent by the HOST */
	HOST_Address = *((uint32_t*) (&Host_Buffer[2]));
	if ((HOST_Address == FIRMWARE_BANK1_BASE)
			|| (HOST_Address == FIRMWARE_BANK2_BASE)) {
		Temp_Firmware.address = HOST_Address;
		Temp_Firmware.crc = 0;
		Temp_Firmware.length = 0;
	}

	/* Extract the payload length from the Host packet */
	Payload_Len = Host_Buffer[6];
	/* Verify the Extracted address to be valid address */
	Address_Verification = Host_Address_Verification(HOST_Address);
	if (ADDRESS_IS_VALID == Address_Verification) {
		/* Write the payload to the Flash memory */
		Flash_Payload_Write_Status = Flash_Memory_Write_Payload(
				(uint8_t*) &Host_Buffer[11], HOST_Address, Payload_Len);
		if (FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status) {
			Temp_Firmware.length += Payload_Len;
			if (Frame_Index == (Total_Frame - 1)) {
				Temp_Firmware.crc = Firmware_CRC_Verification(Temp_Firmware.address, Temp_Firmware.length);
				if (Temp_Firmware.address == FIRMWARE_BANK1_BASE) {
					Firmware1 = Temp_Firmware;
					Firmware1.is_Available = true;
				} else if (Temp_Firmware.address == FIRMWARE_BANK2_BASE) {
					Firmware2 = Temp_Firmware;
					Firmware2.is_Available = true;
				}
				Flash_Write_All_Metadata(&Firmware1, &Firmware2, Firmware_Select);
			}
			/* Report payload write passed */
			Bootloader_Send_Data_To_Host((uint8_t*) &Flash_Payload_Write_Status,
					1);
		} else {
			/* Report payload write failed */
			Bootloader_Send_Data_To_Host((uint8_t*) &Flash_Payload_Write_Status,
					1);
		}
	} else {
		/* Report address verification failed */
		Address_Verification = ADDRESS_IS_INVALID;
		Bootloader_Send_Data_To_Host((uint8_t*) &Address_Verification, 1);
	}

}

/**
 * Kiểm tra địa chỉ hợp lệ trong Flash
 * @param Jump_Address: Địa chỉ cần kiểm tra
 * @return ADDRESS_IS_VALID (1) nếu hợp lệ, ADDRESS_IS_INVALID (0) nếu không
 */
static uint8_t Host_Address_Verification(uint32_t Jump_Address) {
	uint32_t physical_addr = Jump_Address;
//    if (FLASH->OPTCR & FLASH_OPTCR_SWAP_BANK) {
//        if (Jump_Address >= FLASH_BANK1_BASE && Jump_Address < FLASH_BANK2_BASE) {
//            physical_addr += 0x00100000;
//        } else if (Jump_Address >= FLASH_BANK2_BASE && Jump_Address <= STM32F746_FLASH_END) {
//            physical_addr -= 0x00100000;
//        }
//    }
    if ((physical_addr >= FLASH_BASE) && (physical_addr <= STM32F746_FLASH_END)) {
        return ADDRESS_IS_VALID;
    }
    return ADDRESS_IS_INVALID;
}

/**
 * Kiểm tra CRC của dữ liệu
 * @param pData: Dữ liệu cần kiểm tra
 * @param Data_Len: Độ dài dữ liệu
 * @param Host_CRC: Giá trị CRC từ host
 * @return CRC_VERIFICATION_PASSED (1) nếu khớp, CRC_VERIFICATION_FAILED (0) nếu không
 */
static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC) {
    uint8_t CRC_Status = CRC_VERIFICATION_FAILED;
    uint32_t MCU_CRC_Calculated = 0;
    if (Data_Len == 0xFFFFFFFF)
        return MCU_CRC_Calculated;
    CRC->CR = CRC_CR_RESET;
    for (unsigned int i = 0; i < Data_Len; i++)
        CRC->DR = (uint32_t) pData[i];
    if (CRC->DR == Host_CRC) {
        CRC_Status = CRC_VERIFICATION_PASSED;
    } else {
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
void Bootloader_CRC_Verify_Seq(uint8_t *pData, uint32_t Data_Len, uint32_t *InitVal) {
    CRC->CR = CRC_CR_RESET;
    for (unsigned int i = 0; i < Data_Len; i++) {
        CRC->DR = (uint32_t) pData[i];
    }
    *InitVal = CRC->DR;
}

/**
 * Gửi NACK về host
 */
static void Bootloader_Send_NACK(void) {
    uint8_t Ack_Value = CBL_SEND_NACK;
    USART6_send_array((const char*) &Ack_Value, 1);
}

/**
 * Gửi dữ liệu về host
 * @param Host_Buffer: Dữ liệu cần gửi
 * @param Data_Len: Độ dài dữ liệu
 */
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len) {
    USART6_send_array((const char*) Host_Buffer, (uint8_t) Data_Len);
}

/**
 * Reset chip
 */
static void RESET_CHIP(void) {
    uint8_t appExists = 1;
    Bootloader_Send_Data_To_Host((uint8_t*)&appExists, 1);
    HAL_Delay(1);

    while (!rbuffer_empty(&p_USART6_meta->rb_tx));

    __disable_irq();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;

    NVIC_SystemReset();
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
static void Set_Firmware_Version(uint8_t *Host_Buffer) {
    uint8_t status = 0;
    uint8_t Major = Host_Buffer[3];
    uint8_t Minor = Host_Buffer[4];
    uint8_t Patch = Host_Buffer[5];
    if (Host_Buffer[2] == 1) {
        status = 1;
        Firmware1.version_major = Major;
        Firmware1.version_minor = Minor;
        Firmware1.version_patch = Patch;
        Flash_Write_All_Metadata(&Firmware1, &Firmware2, Firmware_Select);
    } else if (Host_Buffer[2] == 2) {
        status = 1;
        Firmware2.version_major = Major;
        Firmware2.version_minor = Minor;
        Firmware2.version_patch = Patch;
        Flash_Write_All_Metadata(&Firmware1, &Firmware2, Firmware_Select);
    }
    Bootloader_Send_Data_To_Host((uint8_t*)&status, 1);
}

/**
 * Lấy phiên bản firmware
 * @param Host_Buffer: Buffer chứa yêu cầu
 */
static void Get_Firmware_Version(uint8_t *Host_Buffer) {
    s_firmware_info *fw_tmp;
    uint8_t tmp[4];
    if (Host_Buffer[2] == 1) {
        tmp[0] = 1;
        fw_tmp = &Firmware1;
    } else if (Host_Buffer[2] == 2) {
        tmp[0] = 1;
        fw_tmp = &Firmware2;
    } else {
        tmp[0] = 0;
    }
    tmp[1] = fw_tmp->version_major;
    tmp[2] = fw_tmp->version_minor;
    tmp[3] = fw_tmp->version_patch;
    Bootloader_Send_Data_To_Host((uint8_t*)tmp, 4);
}

/**
 * Ghi toàn bộ metadata vào Flash
 * @param fw1: Thông tin firmware 1
 * @param fw2: Thông tin firmware 2
 * @param fw_select: Firmware được chọn
 * @return FLASH_OK nếu thành công, FLASH_ERROR nếu lỗi
 */
//static FLASH_StatusTypeDef Flash_Write_All_Metadata(s_firmware_info* fw1, s_firmware_info* fw2, uint8_t fw_select) {
//    uint32_t metadata_bases[] = {METADATA_BANK1_BASE, METADATA_BANK2_BASE};
//    uint8_t banks[] = {FLASH_BANK_1, FLASH_BANK_2};
//
//    uint8_t data[64] __attribute__((aligned(4)));
//    uint32_t fw_info_size = sizeof(s_firmware_info);
//    uint32_t offset = 0;
//    memset(data, 0xFF, sizeof(data));
//
//    memcpy(&data[offset], fw1, fw_info_size);
//    offset += fw_info_size;
//    memcpy(&data[offset], fw2, fw_info_size);
//    offset += fw_info_size;
//    data[offset] = fw_select;
//
//    HAL_FLASH_Unlock();
//
//    for (uint8_t b = 0; b < 2; b++) {
//        uint32_t metadata_addr = metadata_bases[b];
//        uint8_t bank = banks[b];
//
//        // Xóa sector chứa metadata
//        FLASH_EraseInitTypeDef EraseInitStruct = {0};
//        uint32_t SectorError = 0;
//        EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
//        EraseInitStruct.Banks = bank;
//        EraseInitStruct.Sector = 1; // Metadata ở Sector 1
//        EraseInitStruct.NbSectors = 1;
//        EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
//        if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
//            HAL_FLASH_Lock();
//            return FLASH_ERROR;
//        }
//
//        // Ghi metadata từng khối 8 byte
//        for (uint32_t i = 0; i < sizeof(data); i += 4) {
//            __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR);
//            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, metadata_addr + i, *(uint32_t *)&data[i]) != HAL_OK) {
//                HAL_FLASH_Lock();
//                return FLASH_ERROR;
//            }
//        }
//    }
//
//    HAL_FLASH_Lock();
//    return FLASH_OK;
//}
static FLASH_StatusTypeDef Flash_Write_All_Metadata(s_firmware_info* fw1, s_firmware_info* fw2, uint8_t fw_select) {
    uint32_t metadata_bases[] = {METADATA_BANK1_BASE, METADATA_BANK2_BASE};
    uint8_t banks[] = {FLASH_BANK_1, FLASH_BANK_2};

    uint8_t data[64] __attribute__((aligned(4)));
    uint32_t fw_info_size = sizeof(s_firmware_info);
    uint32_t offset = 0;
    memset(data, 0xFF, sizeof(data));

    memcpy(&data[offset], fw1, fw_info_size);
    offset += fw_info_size;
    memcpy(&data[offset], fw2, fw_info_size);
    offset += fw_info_size;
    data[offset] = fw_select;

    HAL_FLASH_Unlock();

    for (uint8_t b = 0; b < 2; b++) {
        uint32_t metadata_addr = metadata_bases[b];
        uint8_t bank = banks[b];

        FLASH_EraseInitTypeDef EraseInitStruct = {0};
        uint32_t SectorError = 0;
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
        EraseInitStruct.Banks = bank;
        EraseInitStruct.Sector = 1; // Metadata ở Sector 1
        EraseInitStruct.NbSectors = 1;

        if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
            HAL_FLASH_Lock();
            return FLASH_ERROR;
        }

        for (uint32_t i = 0; i < sizeof(data); i += 4) {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, metadata_addr + i, *(uint32_t *)&data[i]) != HAL_OK) {
                HAL_FLASH_Lock();
                return FLASH_ERROR;
            }
        }
    }

    HAL_FLASH_Lock();
    return FLASH_OK;
}


/**
 * Đọc toàn bộ metadata từ Flash
 * @param fw1: Thông tin firmware 1
 * @param fw2: Thông tin firmware 2
 * @param fw_select: Firmware được chọn
 * @param bank_select: Bank cần đọc
 * @return FLASH_OK nếu thành công, FLASH_ERROR nếu lỗi
 */
static FLASH_StatusTypeDef Flash_Read_All_Metadata(s_firmware_info* fw1, s_firmware_info* fw2, uint8_t* fw_select, uint8_t bank_select) {
    uint32_t metadata_bases[] = {METADATA_BANK1_BASE, METADATA_BANK2_BASE};
    uint8_t swapped = 0;

    if (bank_select > 1) return FLASH_ERROR;

    uint32_t metadata_addr = metadata_bases[bank_select ^ swapped];
    uint8_t data[32] __attribute__((aligned(4)));
    uint32_t fw_info_size = sizeof(s_firmware_info);
    uint32_t offset = 0;

    memcpy(data, (void*)metadata_addr, sizeof(data));

    memcpy(fw1, &data[offset], fw_info_size);
    offset += fw_info_size;
    memcpy(fw2, &data[offset], fw_info_size);
    offset += fw_info_size;
    *fw_select = data[offset];

    if(fw1->address == 0xffffffff)
    {
    	fw1->is_Available = false;
    }
    if(fw2->address == 0xffffffff)
	{
		fw2->is_Available = false;
	}
    return FLASH_OK;
}

/**
 * Kiểm tra firmware có sẵn và nhảy đến nếu hợp lệ
 */
void Firmware_Check_Available(void) {
    uint32_t CRC_Result = 0;

    Flash_Read_All_Metadata(&Firmware1, &Firmware2, &Firmware_Select, 0);

    uint8_t swapped = 0;
    // Kiểm tra Firmware 1
    if (Firmware1.is_Available) {
        if (Firmware1.length > 0) {
            CRC_Result = Firmware_CRC_Verification(Firmware1.address, Firmware1.length);
        }

        if (Firmware1.length == 0 || CRC_Result == Firmware1.crc) {
            Firmware_Select = 1;
            Jump_To_App(Firmware1.address);
            return;
        }
    }

    // Kiểm tra Firmware 2
    if (Firmware2.is_Available) {
        if (Firmware2.length > 0) {
            CRC_Result = Firmware_CRC_Verification(Firmware2.address, Firmware2.length);
        }

        if (Firmware2.length == 0 || CRC_Result == Firmware2.crc) {
            Firmware_Select = 2;
            Jump_To_App(Firmware2.address);
            return;
        }
    }
}
