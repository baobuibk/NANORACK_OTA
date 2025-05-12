# Python Host for STM32 FOTA

## Overview
The Python host script (`FOTA.py`) is a command-line tool for managing firmware updates on STM32 microcontrollers (STM32F407VGTx, STM32F765VGTx, STM32H745ZIT3) via UART.
## Features
- **Two Operation Modes**:
  - **Option Mode**: Interactive menu for manual control of bootloader operations.
  - **Sequential Mode**: Automated firmware update via command-line arguments.
- [Command Protocol](../STM32_BOOTLOADER.md#implementation-details)
- Supports single-core (STM32F407VGTx, STM32F765VGTx) and dual-core (STM32H745ZIT3) MCUs.
- Firmware version management (e.g., 1.0.0 format).
- CRC32-based data integrity verification.
- UART console mode for real-time interaction with the application.
- Automatic detection and listing of `.bin` files in the current directory.
## Requirements
- **Python Version**: 3.8 or higher.
- **Libraries**:
  - `pyserial`: For UART communication.
  - `tqdm`: For progress bar visualization.
- **Hardware**:
  - UART-to-USB adapter (e.g., FT232R).
- **Firmware Files**: `.bin` files for flashing.

## Installation
1. Install Python dependencies:
   ```bash
   pip install -r requirements.txt
   ```
2. Place the `FOTA.py` script in the project directory.
3. Ensure `.bin` firmware files are available in the same directory or a specified path.

## Code Structure
The Python script is organized as follows:

- **Imports and Constants**:
  - `CRC_TABLE`: Precomputed CRC32 MPEG-2 table for data integrity checks.
- **Utility Functions**:
  - `calculate_crc32(data)`: Computes CRC32 checksum for packets.
  - `list_bin_files()`: Lists and allows selection of `.bin` files.
- **STM32Bootloader Class**:
  - Manages UART communication and bootloader commands.
  - Methods:
    - `connect_serial()`: Establishes UART connection.
    - `check_connection()`: Verifies bootloader responsiveness.
    - `read_chip_id()`: Retrieves MCU chip ID.
    - `select_firmware(mode)`: Selects firmware bank (1 or 2).
    - `erase_flash()`: Erases specified flash bank.
    - `upload_application(bin_path)`: Uploads firmware in 128-byte chunks.
    - `write_firmware_version(version)`: Sets firmware version.
    - `read_firmware_detail()`: Reads firmware metadata (version, size, status).
    - `jump_to_application()`: Jumps to the flashed application.
    - `uart_terminal()`: Provides real-time UART console.
    - `close()`: Closes UART connection.
- **Main Functions**:
  - `process_opt_mode()`: Implements the interactive menu.
  - `process_seq_mode(args)`: Handles automated firmware update.
  - `validate_fw_args(args)`: Validates command-line arguments.
  - `main()`: Parses arguments and dispatches to the appropriate mode.

## Usage
### Option Mode
Run the script in interactive mode:
```bash
python FOTA.py -mode opt
```
- **Steps**:
  1. Select a COM port from the listed options.
  2. Choose MCU type (F4, F7, or H7).
  3. Use the menu to perform operations:
     - Check connection.
     - Read chip ID.
     - Select firmware (1 or 2 for dual-core).
     - Erase flash.
     - Select and upload a `.bin` file.
     - Set firmware version.
     - Read firmware details.
     - Jump to application.
     - Enter UART console mode.
     - Exit.

### Sequential Mode
Run the script with arguments for automated flashing:

- For single-core (STM32F407VGTx, STM32F765VGTx):
  ```bash
  python FOTA.py -mode seq -port COM5 -mcu F4 -bin firmware1.bin -v 1.0.0
  ```
- For dual-core (STM32H745ZIT3):
  ```bash
  python fota_host.py -mode seq -port COM5 -mcu H7 -bin1 firmware1.bin -bin2 firmware2.bin -v1 1.0.0 -v2 1.0.0
  ```
- **Arguments**:
  - `-mode`: `opt` or `seq`.
  - `-port`: COM port (e.g., COM5 or /dev/ttyUSB0).
  - `-mcu`: MCU type (F4, F7, or H7).
  - `-bin`: Path to `.bin` file (for F4/F7).
  - `-bin1`, `-bin2`: Paths to `.bin` files for H7 (Bank 1 and Bank 2).
  - `-v`, `-v1`, `-v2`: Firmware version(s) in `x.y.z` format.

## Error Handling
- **Serial Connection Errors**: Checks for valid COM ports and handles connection failures.
- **File Errors**: Validates `.bin` file existence and format.
- **Version Format**: Ensures version strings follow `x.y.z` format.
- **Bootloader Communication**: Retries and reports failures (e.g., NACK responses).

## Notes
- The script uses a baud rate of 115200, matching the bootloader configuration.
- The UART console mode supports real-time interaction with the application (e.g., sending "help" commands).
- CRC32 verification ensures data integrity during transmission.
- In Sequential Mode, if version's argument is not given, the default version will be 1.0.0.

## Limitations
- Requires manual COM port selection in Option Mode.
- No support for non-UART communication (e.g., CAN, SPI).
- Assumes `.bin` files are correctly aligned with the STM32 memory map.
