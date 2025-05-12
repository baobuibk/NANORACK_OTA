import serial
import time
import os
import struct
import threading
import sys
import argparse
import re
from tqdm import tqdm
import serial.tools.list_ports
# Bảng CRC32 MPEG-2 từ firmware ESP32
CRC_TABLE = [
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF, 0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4,
]

def calculate_crc32(data):
    
    checksum = 0xFFFFFFFF
    crc_packet = bytearray()  

    for byte in data:
        crc_packet.extend([0x00, 0x00, 0x00, byte])

    for byte in crc_packet:
        top = (checksum >> 24) & 0xFF
        top ^= byte
        checksum = ((checksum << 8) & 0xFFFFFFFF) ^ CRC_TABLE[top]

    return checksum

def list_bin_files():
    # Lấy thư mục hiện tại của file Python
    current_dir = os.path.dirname(os.path.abspath(__file__))
    # Liệt kê tất cả file .bin trong thư mục hiện tại
    bin_files = [f for f in os.listdir(current_dir) if f.endswith('.bin')]
    
    if not bin_files:
        print("No .bin files found in the current directory!")
    else:
        print("Available .bin files:")
        for index, file in enumerate(sorted(bin_files), start=1):
            print(f"{index}: {file}")
    
    while True:
        try:
            if bin_files:
                choice = input("Select a .bin file number or enter file path: ")
            else:
                choice = input("Enter file path: ")
            # Kiểm tra nếu nhập số thứ tự
            try:
                choice_num = int(choice)
                if (not bin_files) and choice_num: 
                    print("Invalid choice. Please enter file path!")
                else:
                    if 1 <= choice_num <= len(bin_files):
                        return os.path.join(current_dir, bin_files[choice_num - 1])
                    print(f"Invalid choice. Please select 1 to {len(bin_files) if bin_files else 0} or a valid file path.")
            except ValueError:
                # Kiểm tra nếu nhập đường dẫn file
                file_path = choice.strip().strip('"\' ')
                if os.path.isfile(file_path) and file_path.endswith('.bin') and os.path.exists(file_path):
                    return file_path
                print("Invalid file path or file is not a .bin file.")
        except KeyboardInterrupt:
            print("\nOperation cancelled by user.")
            return None



class STM32Bootloader:
    def __init__(self, port, mcu, baudrate=115200):
        self.mcu = mcu
        self.port = port
        self.baudrate = baudrate
        self.serial = None
        self.firmware_number = None  # Store firmware selection
        self.NOT_ACKNOWLEDGE = 0xAB
        self.JUMP_SUCCEEDED = 0x01
        self.ERASE_SUCCEEDED = 0x01
        self.WRITE_SUCCEEDED = 0x01
        self.RESET_SUCCEEDED = 0x01
        self.READ_VERSION_SUCCEEDED = 0x01
        self.WRITE_VERSION_SUCCEEDED = 0x01

    def read_firmware_detail(self):
        """Read Firmware Version (0x16)"""
        if not self.firmware_number:
            print("No firmware selected! Please choose firmware first.")
            return False
        print(f"\rReading firmware {self.firmware_number}'s data...", end='')
        packet = bytearray(7)
        packet[0] = 6  # Packet length
        packet[1] = 0x16  # Read Version command
        packet[2] = self.firmware_number & 0xFF  # Firmware number (1 or 2)
        crc = calculate_crc32(packet[:3])
        struct.pack_into('<I', packet, 3, crc)
        
        response = self.send_packet(packet, 7, 10)  # Expecting 1 status byte + 1 avaiable + 3 version bytes
        if response and response[0] == self.READ_VERSION_SUCCEEDED and len(response) == 7:
            print("   \033[32mDone ✓\033[0m")
            print(f"Firmware {self.firmware_number} Version: {response[1]}.{response[2]}.{response[3]}")
            print(f"Firmware {self.firmware_number} Size: {(response[5]<< 8) | response[6]}KB")
            print(f"Firmware {self.firmware_number} Status: {'Available' if response[4] != 0 else 'Unavailable'}")
            return True
        print("   \033[31mFailed ✗\033[0m")# to read firmware data!")
        return False
    
    def write_firmware_version(self, version_name):
        """Write Firmware Version (0x17)"""
        if not self.firmware_number:
            print("No firmware selected! Please choose firmware first.")
            return False
        version_name = version_name.strip()
        major, minor, patch = map(int, version_name.split('.'))

        print(f"\rWriting firmware {self.firmware_number} version {major}.{minor}.{patch}...", end='')
        packet = bytearray(10)
        packet[0] = 9  # Packet length
        packet[1] = 0x17  # Write Version command
        packet[2] = self.firmware_number & 0xFF  # Firmware number (1 or 2)
        packet[3] = major & 0xFF
        packet[4] = minor & 0xFF
        packet[5] = patch & 0xFF
        crc = calculate_crc32(packet[:6])
        struct.pack_into('<I', packet, 6, crc)
        
        response = self.send_packet(packet, 1, 20)
        if response and response[0] == self.WRITE_VERSION_SUCCEEDED:
            print("   \033[32mDone ✓\033[0m")#Write Firmware Version Successful")
            return True
        print("   \033[31mFailed ✗\033[0m")#Write Firmware Version Failed")
        return False
    
    def connect_serial(self):
        """Connect to the STM32 bootloader via UART"""
        try:
            self.serial = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                bytesize=serial.EIGHTBITS,
                timeout=2
            )
            print(f"Connected to STM32 on {self.port.upper()}")
            return True
        except serial.SerialException as e:
            print(f"Serial connection error: {e}")
            return False

    def send_packet(self, packet, response_length, time_out):
        """Send a packet and wait for response"""
        self.serial.reset_input_buffer()
        self.serial.reset_output_buffer()
        self.serial.write(packet)
        start_time = time.time()
        while time.time() - start_time < time_out:  # Timeout
            if self.serial.in_waiting >= response_length:
                return self.serial.read(response_length)
        return None

    def read_chip_id(self):
        """Read Chip ID (0x10)"""
        packet = bytearray(6)
        packet[0] = 5
        packet[1] = 0x10
        crc = calculate_crc32(packet[:2])
        struct.pack_into('<I', packet, 2, crc)
        response = self.send_packet(packet, 2, 1)
        if response and response[0] != self.NOT_ACKNOWLEDGE:
            chip_id = (response[1] << 8) | response[0]
            print(f"Chip ID: 0x{chip_id:04X}")
            return True
        print("Failed to read Chip ID!")
        return False
    
    def check_connection(self):
        """Send 'bootmode' command to bootloader to jump to boot mode"""
        print("\rChecking connection...", end='')
        packet = bytearray([7, ord('b'), ord('t'), ord('m'), ord('o'), ord('d'), ord('e'), ord('\r')])
        self.serial.reset_input_buffer()
        self.serial.reset_output_buffer()
        self.serial.write(packet)
        start_time = time.time()
        response = bytearray()
        while time.time() - start_time < 5:
            if  self.serial.in_waiting > 0:
                response.extend( self.serial.read( self.serial.in_waiting))
                # Kiểm tra từng byte
                for i in range(len(response)):
                    if i + 2 < len(response):  # Đảm bảo đủ 3 byte
                        if (response[i] == 1 and response[i + 1] == ord('O') and response[i + 2] == ord('K')):
                            print("   \033[32mDone ✓\033[0m")
                            time.sleep(0.6)
                            return True
            
            time.sleep(0.01)  # Ngủ ngắn để giảm tải CPU
        print("   \033[31mFailed ✗\033[0m")
        #print("   \033[32mDone ✓\033[0m")
        #time.sleep(0.6)
        return False

    def select_firmware(self, mode):
        """Select firmware number (1 or 2)"""
        if mode == 0:
            while True:
                choice = input("Select firmware number (1 or 2): ")
                if self.mcu == 1:         #mcu is f4 or f7 (1 core)
                    if choice == "1":
                        self.firmware_number = int(choice)
                        print(f"Firmware {self.firmware_number} selected")
                        return True
                    else:
                        print("Invalid input! MCU F4, F7 have 1 core, pls enter 1.")
                elif self.mcu == 2:         #mcu is h7 (2 core) 
                    if choice in ["1", "2"]:
                        self.firmware_number = int(choice)
                        print(f"Firmware {self.firmware_number} selected")
                        return True
                    print("Invalid input! Please enter 1 or 2.")
        else:
            self.firmware_number = mode 
            
            

    def erase_flash(self):
        """Erase Flash (0x13)"""
        if not self.firmware_number:
            print("No firmware selected! Please choose firmware first.")
            return False
        packet = bytearray(7)
        packet[0] = 6
        packet[1] = 0x13
        packet[2] = self.firmware_number & 0xFF
        
        crc = calculate_crc32(packet[:3])
        struct.pack_into('<I', packet, 3, crc)
        print(f"\rErasing firmware {self.firmware_number}...", end='')
        response = self.send_packet(packet, 1, 20)
        if response and response[0] == self.ERASE_SUCCEEDED:
            print("   \033[32mDone ✓\033[0m")#Erase Flash Successful")
            return True
        print("   \033[31mFailed ✗\033[0m")
        return False

    def upload_application(self, bin_path):
        """Upload firmware via UART"""
        if not self.firmware_number:
            print("No firmware selected! Please choose firmware (1 or 2) first.")
            return False
        if not os.path.exists(bin_path):
            print(f"File not found: {bin_path}")
            return False

        with open(bin_path, 'rb') as f:
            firmware_data = f.read()

        file_size = len(firmware_data)
        chunk_size = 128
        total_frames = (file_size + chunk_size - 1) // chunk_size
        frame_index = 0

        print(f"Firmware-Size: {file_size} bytes, Total frames: {total_frames}")
        print(f"Uploading firmware {self.firmware_number}:")
        for i in tqdm(range(0, file_size, chunk_size), desc="Processing", unit="frame", ncols=80):
        
        
        #for i in range(0, file_size, chunk_size):
            actual_chunk_size = min(chunk_size, file_size - i)
            packet_length = actual_chunk_size + 12  # 8 (header) + 4 (CRC)

            packet = bytearray(packet_length)
            packet[0] = packet_length - 1  # Độ dài gói (trừ đi byte này)
            packet[1] = 0x14  # Mã lệnh Upload Application
            packet[2] = self.firmware_number & 0xFF  # Firmware number (1 or 2)
            packet[3] = actual_chunk_size  # Chunk size

            struct.pack_into('<H', packet, 4, frame_index)  
            struct.pack_into('<H', packet, 6, total_frames)  

            chunk = firmware_data[i:i + actual_chunk_size]
            packet[8:8 + actual_chunk_size] = chunk  # Payload

            crc = calculate_crc32(packet[:8 + actual_chunk_size])
            struct.pack_into('<I', packet, 8 + actual_chunk_size, crc)  # CRC32
            #print(f"Send frame {frame_index + 1}/{total_frames}, Size: {actual_chunk_size} bytes")

            response = self.send_packet(packet, 1, 20)
            if not response or response[0] != self.WRITE_SUCCEEDED:
                print("\r\n\033[31mUpload failed!\033[0m")
                return False

            progress = int((i + actual_chunk_size) / file_size * 100)
            #print(f"Process: {progress:.1f}%")
            frame_index += 1

        print("\033[32mDone ✓\033[0m")
        return True

    def jump_to_application(self):
        """Jump to application (0x12)"""
        print("\rJumping to Application...", end='')
        packet = bytearray(6)
        packet[0] = 5  # Độ dài gói tin
        packet[1] = 0x12  # Lệnh Jump
        
        crc = calculate_crc32(packet[:2])  # CRC trên 2 byte đầu
        struct.pack_into('<I', packet, 2, crc)  # Nhúng CRC vào packet
        response = self.send_packet(packet, 1, 5)
        if response and response[0] == self.JUMP_SUCCEEDED:
            print("   \033[32mDone ✓\033[0m")
            return True
        print("   \033[31mFailed ✗\033[0m")
        return False
    
    def reset_chip(self):
        """Reset STM32 chip (0x15)"""
        print("Resetting STM32 chip...")
        packet = bytearray(6)
        packet[0] = 5
        packet[1] = 0x15
        crc = calculate_crc32(packet[:2])
        struct.pack_into('<I', packet, 2, crc)
        response = self.send_packet(packet, 1, 1)
        if response and response[0] != self.NOT_ACKNOWLEDGE:
            print("STM32 Reset Successful")
            return True
        print("STM32 Reset Failed")
        return False

    def uart_terminal(self):
    
        print("STM32 Console. Enter commands (Ctrl+C to exit).")
        self.serial.reset_input_buffer()
        self.serial.reset_output_buffer()
        try:
            while True:
                # Nhập lệnh từ người dùng
                command = input(' ').strip()     
                # Gửi lệnh với \r
                self.serial.write((command + '\r').encode('utf-8'))
                
                # Đợi và nhận phản hồi
                time.sleep(0.1)  # Đợi 100ms để STM32 xử lý
                response = ""
                start_time = time.time()
                while time.time() - start_time < 1:  # Timeout 1 giây
                    if self.serial.in_waiting:
                        response += self.serial.read(self.serial.in_waiting).decode('utf-8', errors='ignore')
                        if ">" in response:  # Dừng khi nhận được dấu nhắc
                            break
                    time.sleep(0.01)
                
                # Hiển thị phản hồi, loại bỏ dấu nhắc "> "
                if response:
                    print(response.strip(command).rstrip(), end='')
                self.serial.reset_input_buffer()
                self.serial.reset_output_buffer()
        
        except KeyboardInterrupt:
            return
        finally:
            return

    def close(self):
        """Close the serial connection"""
        if self.serial and self.serial.is_open:
            self.serial.close()
            print("Serial connection closed")


####################################################################################
############################## BOOTLOADER OPTION MODE ##############################
####################################################################################

def process_opt_mode():
    
    print("Welcome to STM32 Bootloader Menu")
    #port = input("Enter COM port (e.g., COM5 or /dev/ttyUSB0): ")
    ports = sorted(serial.tools.list_ports.comports(), key=lambda x: x.device)
    if not ports:
        print("No COM ports found!")
        return
    
    while True:
        try:
            print("Available COM ports:")
            for index, port in enumerate(ports, start=1):
                print(f"{index}: {port.description}")
            choice = input("Select an option: ")
            choice = int(choice)
            if 1 <= choice <= len(ports):
                port = ports[choice - 1]
                break
            raise ValueError
            #print(f"Invalid choice. Please select 1 to {len(ports)}.")
        except ValueError as e:
            print(f"Invalid choice. Please select 1 to {len(ports)}.")
        except KeyboardInterrupt:
            print("\nOperation cancelled by user.")
            return
    while True:
        try:
            print("Choose your mcu selection:")
            print("1: STM32F4, STM32F7 (single core)")
            print("2: STM32H7 (dual core)")
            choice = input("Select an option: ")
            choice = int(choice)
            if 1 <= choice <= 2:
                mcu = choice
                print(f"Device {"STM32F4, STM32F7" if mcu == 1 else "STM32H7"} selected.")
                break
            raise ValueError
            #print("Invalid choice. Please select 1 to 2.")
        except ValueError as e:
            print(f"Invalid choice. Please select 1 to 2.")
        except KeyboardInterrupt:
            print("\nOperation cancelled by user.")
            return
    '''    mcu_type = input("Enter type of MCU (e.g., F4, F7, H7): ")
        if mcu_type in ["F4", "F7", "f4", "f7"]:
            mcu = 1
        elif mcu_type in ["H7", "h7"]:
            mcu = 2
        else:
            print("Error: Invalid MCU type. Must be 'F4', 'F7', or 'H7'.")
            return
    '''
        
    bootloader = STM32Bootloader(port.device, mcu)
    if not bootloader.connect_serial():
        return
    
    bin_file = None
    connection_checked = False 
    
    while True:
        try:
            print("\nMenu Options:")
            print("1: Check Connection")
            print("2: Read Chip ID")
            print("3: Select Firmware (1 or 2)")
            print("4: Erase Flash")
            print("5: Select Firmware File")
            print("6: Flash Firmware")
            print("7: Write Firmware Version")
            print("8: Read Firmware Detail")
            print("9: Jump to Application")
            print("10: UART Console")
            print("11: Exit")

            choice = input("Select an option (1-11): ")
            if choice in ["2", "3", "4", "5", "6", "7", "8", "9"]:
                if not connection_checked:
                    print("Error: Please select 'Check Connection' (option 1) first.")
                    continue
                
            if choice == "1":
                if bootloader.check_connection():
                    None
                if bootloader.check_connection():
                    connection_checked = True      
            elif choice == "2":
                bootloader.read_chip_id()
            elif choice == "3":
                bootloader.select_firmware(0)   # select fw_number using input
            elif choice == "4":
                bootloader.erase_flash()
            elif choice == "5":
                bin_file = list_bin_files()
                if not bin_file:
                    bootloader.close()
                    sys.exit(0)
                else:
                    print(f"Selected file: {bin_file}")
            elif choice == "6":
                if bin_file:
                    bootloader.upload_application(bin_file)
                else:
                    print("No bin file selected! Please select a file first.")
            elif choice == "7":
                while True:
                    try:
                        ver = input("Enter version name (e.g., 1.0.0): ").strip()
                        if not re.match(r"^\d+\.\d+\.\d+$", ver):
                            raise ValueError(f"{ver} must follow 'x.y.z' format (e.g., 1.0.0).")
                        break    
                    except ValueError as e:
                        print(e)
                bootloader.write_firmware_version(ver)
            elif choice == "8":
                bootloader.read_firmware_detail()
            elif choice == "9":
                if bootloader.jump_to_application():
                    connection_checked = False
            elif choice == "10": 
                if connection_checked:
                    print("This selection is used for console mode in Application! Please select 'Jump to Application' (option 9) to use this selection.")
                else:
                    bootloader.uart_terminal()
            elif choice == "11":
                print("Exiting...")
                bootloader.close()
                break
            else:
                print("Invalid choice, please try again.")

        except KeyboardInterrupt:
            bootloader.close()
            print("\nKeyboard Interrupt detected. Exiting safely...")
            sys.exit(0)
    
    
####################################################################################
############################ BOOTLOADER SEQUENCE MODE ##############################
####################################################################################

def process_seq_mode(args):
    """
    Process arguments for sequential mode.
    """
    print("Processing in Sequential Mode:")
    
    if args.mcu in ["F4", "F7", "f4", "f7"]:
        mcu = 1
    elif args.mcu in ["H7", "h7"]:
        mcu = 2
        
    bootloader = STM32Bootloader(args.port, mcu)
    if not bootloader.connect_serial():
        return
    

    if mcu == 1:
        args.bin = args.bin.strip()
        if not os.path.exists(args.bin):
            print("Path not found! Try again.")
        else:
            if not args.bin.lower().endswith('.bin'):
                print("Path must end with '.bin'.")
                return
            print(f"Path firmware: {args.bin}")
    elif mcu == 2:
        args.bin1 = args.bin1.strip()
        args.bin2 = args.bin2.strip()
        if not os.path.exists(args.bin1):
            print("ERROR: Path firmware 1 not found! Try again.")
        else:
            if not args.bin1.lower().endswith('.bin'):
                print("ERROR: Path firmware 1 must end with '.bin'!")
                return
            print(f"Path firmware 1: {args.bin1}")
            
        if not os.path.exists(args.bin2):
            print("ERROR: Path firmware 2 not found! Try again.")
        else:
            if not args.bin2.lower().endswith('.bin'):
                print("ERROR: Path firmware 2 must end with '.bin'!")
                return
            print(f"Path firmware 2: {args.bin2}")
    

    '''Step 1: Check connection: jump to bootloader'''
    if not bootloader.check_connection():
        bootloader.close()
        sys.exit(1)
    if not bootloader.check_connection():
        bootloader.close()
        sys.exit(1)
    time.sleep(0.2)
    '''Step 2: Select firmware 1'''
    bootloader.select_firmware(1)
    
    '''Step 3: Erase Flash bank 1'''
    if not bootloader.erase_flash():
        bootloader.close()
        sys.exit(1)
        
    '''Step 4: Flash firmware 1'''    
    if mcu == 1:
        if not bootloader.upload_application(args.bin):
            bootloader.close()
            sys.exit(1)
    else:
        if not bootloader.upload_application(args.bin1):
            bootloader.close()
            sys.exit(1)
    
    '''Step 5: Flash metadata 1'''
    if mcu == 1:
        if not bootloader.write_firmware_version(args.v):
            bootloader.close()
            sys.exit(1)
    else:
        if not bootloader.write_firmware_version(args.v1):
            bootloader.close()
            sys.exit(1)
    
    '''Step 6: Firmware 2 if '''
    if mcu == 2:
        '''Select firmware 2 (option)'''
        bootloader.select_firmware(2)    
        
        '''Erase Flash bank 2 (option)'''
        if not bootloader.erase_flash():
            bootloader.close()
            sys.exit(1)
            
        '''Flash firmware 2 (option)'''    
        if not bootloader.upload_application(args.bin2):
            bootloader.close()
            sys.exit(1)
        
        '''Flash metadata 2 (option)'''
        if not bootloader.write_firmware_version(args.v2):
            bootloader.close()
            sys.exit(1)
    
    '''Step 7: Jump to application'''
    if not bootloader.jump_to_application():
        bootloader.close()
        sys.exit(1)
    
####################################################################################
####################################################################################
####################################################################################    
    
    
def validate_fw_args(args):
    # Check port
    if not args.port:
        raise argparse.ArgumentError(None, "Port argument (-port) is required.")
    
    # Check bin file for MCU
    if args.mcu in ["F4", "F7", "f4", "f7"]:
        if not args.bin:
            raise argparse.ArgumentError(None, f"For MCU {args.mcu}, exactly one -bin argument is required.")
        if args.bin1 or args.bin2:
            raise argparse.ArgumentError(None, f"For MCU {args.mcu}, use -bin, not -bin1 or -bin2.")
        if not re.match(r"^\d+\.\d+\.\d+$", args.v):
            raise argparse.ArgumentError(None,  "Firmware's vesion must follow 'x.y.z' format (e.g., 1.0.0).")
    elif args.mcu in ["H7", "h7"]:
        if not (args.bin1 and args.bin2):
            raise argparse.ArgumentError(None, "For MCU H7, both -bin1 and -bin2 arguments are required.")
        if args.bin:
            raise argparse.ArgumentError(None, "For MCU H7, use -bin1 and -bin2, not -bin.")
        if not re.match(r"^\d+\.\d+\.\d+$", args.v1):
            raise argparse.ArgumentError(None, "Firmware 1's vesion must follow 'x.y.z' format (e.g., 1.0.0).")
        if not re.match(r"^\d+\.\d+\.\d+$", args.v2):
            raise argparse.ArgumentError(None, "Firmware 2's vesion must follow 'x.y.z' format (e.g., 1.0.0).")
    else:
        raise argparse.ArgumentError(None, "Invalid MCU type. Must be 'F4', 'F7', or 'H7'.")
    
    

def main():
    parser = argparse.ArgumentParser(description="FOTA update for STM32 MCU")
    parser.add_argument("-mode", type=str, required=True, choices=["opt", "seq"], help="Option mode: 'option' for interactive menu, 'seq' for sequential FOTA")
    parser.add_argument("-port", type=str, help="Serial port (e.g., COMx or /dev/ttyUSB0)")
    parser.add_argument("-mcu", type=str, help="MCU type (F4 or F7 or H7)")
    parser.add_argument("-bin", type=str, help="Single .bin file for F4 & F7")
    parser.add_argument("-bin1", type=str, help="First .bin file for H7")
    parser.add_argument("-bin2", type=str, help="Second .bin file for H7")
    parser.add_argument("-v", type=str, default="1.0.0", help="Version string (e.g., 1.0.0)")
    parser.add_argument("-v1", type=str, default="1.0.0", help="Version string (e.g., 1.0.0)")
    parser.add_argument("-v2", type=str, default="1.0.0", help="Version string (e.g., 1.0.0)")
    try:
        args = parser.parse_args()
        
        # Process based on mode
        if args.mode == "seq":
            # Validate arguments
            validate_fw_args(args)
            process_seq_mode(args)
        elif args.mode == "opt":
            process_opt_mode()
            
    except argparse.ArgumentError as e:
        print(f"Error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {e}")
        sys.exit(1)
        


if __name__ == "__main__":
    main()
