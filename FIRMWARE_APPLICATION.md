# STM32 Application for FOTA

## Overview
This document describes the requirements and structure for developing applications compatible with the STM32H745ZIT3 bootloader. A sample application is provided for both Cortex-M7 and Cortex-M4 cores (for STM32H7), demonstrating how to initialize the system and interact with the bootloader. The application must be compiled as a `.bin` file and flashed to the correct memory address (`FIRMWARE_BANK1_BASE` or `FIRMWARE_BANK2_BASE`).

## Features
- **UART Interaction**: Supports console commands (e.g., "reset") via UART.
- **Simple Demo**: Toggles GPIO pins to demonstrate functionality.

## Requirements
- **Hardware**:
  - STM32 development board (STM32F4, STM32F7, or STM32H7).
  - UART interface for console output.
- **Software**:
  - STM32CubeIDE for compilation.
  - STM32CubeProgrammer for generating `.bin` files.
- **Bootloader**: Must be flashed and configured correctly.

## Memory Map
Applications must be linked to the following addresses:
- **Firmware 1**: `0x08040000` (Bank 1, Sectors 2-5).
- **Firmware 2** (STM32H7 only): `0x08140000` (Bank 2, Sectors 2-5).

The linker script must set the vector table to these addresses, and the `.bin` file must start at the specified address.

## Sample Application
The provided sample application includes:
- **main_m7.c** (Cortex-M7):
  - Initializes system clock, GPIO, and UART.
  - Runs a scheduler to handle tasks (e.g., processing UART commands).
  - Supports console commands via `command_init()` (not provided but assumed).
- **main_m4.c** (Cortex-M4):
  - Synchronizes with Cortex-M7 using HSEM.
  - Toggles a GPIO pin (PD7) every 100ms as a demo.

### Key Code Snippets
#### Cortex-M7 (`main_m7.c`)
```c
int main(void) {
  MPU_Config();
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  command_init();
  while (1) {
    SchedulerRun();
  }
}
```
- Initializes peripherals and runs the scheduler.
- The scheduler handles UART command processing (via `command_init()`).

#### Cortex-M4 (`main_m4.c`)
```c
int main(void) {
  __HAL_RCC_HSEM_CLK_ENABLE();
  HAL_HSEM_FastTake(0);
  HAL_Init();
  MX_GPIO_Init();
  while (1) {
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_7);
    HAL_Delay(100);
  }
}
```
- Synchronizes with M7 using HSEM.
- Toggles a GPIO pin as a simple demo.

## Developing a Compatible Application
To ensure the application works with the bootloader, follow these guidelines:

1. **Memory Layout**:
   - Set the linker script to place the vector table at `FIRMWARE_BANK1_BASE` (`0x08040000`) or `FIRMWARE_BANK2_BASE` (`0x08140000`).
   - Example linker script (STM32CubeIDE):
     ```ld
     MEMORY {
       FLASH (rx) : ORIGIN = 0x08040000, LENGTH = 512K
       RAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 128K
     }
     ```
   - Set `SCB->VTOR = FIRMWARE_BANK1_BASE` in the application if needed.

2. **Vector Table**:
   - Ensure the vector table includes a valid reset handler.
   - The bootloader jumps to the address stored at `app_address + 4` (reset handler).

3. **System Initialization**:
   - Initialize the system clock, peripherals, and HAL in `main()`.
   - For STM32H7, the Cortex-M4 must synchronize with Cortex-M7 using HSEM:
     ```c
     __HAL_RCC_HSEM_CLK_ENABLE();
     HAL_HSEM_FastTake(0); // Take semaphore for M4
     ```

4. **UART Support**:
   - If the application uses UART (e.g., for console), configure it to match the bootloader (115200 baud, 8-N-1, PA2: TX, PD6: RX).
   - Implement command handling (e.g., "reset" to return to bootloader mode).

5. **Reset Command**:
   - To return to bootloader mode, implement a UART command (e.g., "reset"):
     ```c
     if (strcmp(command, "reset") == 0) {
       NVIC_SystemReset();
     }
     ```
   - The bootloader will handle the reset and enter bootloader mode if a connection is detected.

6. **Binary Generation**:
   - Compile the application and generate a `.bin` file using STM32CubeProgrammer or `objcopy`:
     ```bash
     arm-none-eabi-objcopy -O binary app.elf app.bin
     ```
   - Ensure the `.bin` file starts at the correct address (`0x08040000` or `0x08140000`).

## Compilation and Flashing
1. **Create Project**:
   - Use STM32CubeIDE to create an application project.
   - Include `main_m7.c` and `main_m4.c` (for H7).
2. **Configure Linker**:
   - Update the linker script to match the memory map.
3. **Build and Generate Binary**:
   - Compile the project.
   - Generate `.bin` file.
4. **Flash via FOTA**:
   - Use the Python host script to upload the `.bin` file:
     ```bash
     python fota_host.py -mode seq -port COM5 -mcu H7 -bin1 app.bin -v 1.0.0
     ```

## Notes
- The application must not overwrite the bootloader or metadata regions.
- For STM32H7, ensure both M7 and M4 applications are synchronized via HSEM.
- Test the application with the UART console to verify functionality.

## Limitations
- The sample application is minimal and may need expansion for real-world use.
- No support for secure boot or encrypted firmware.

For details on the Python host and bootloader, refer to `README_HOST.md` and `README_BOOTLOADER.md`.