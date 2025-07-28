# STM32 Application for FOTA

## Overview
This document outlines the requirements and structure for developing applications compatible with the STM32H745ZIT3 (OBC, dual-core) and STM32F765VGTx (EXP, single-core) bootloaders. Sample applications are provided for Cortex-M7 and Cortex-M4 cores (OBC) and single-core Cortex-M7 (EXP), demonstrating system initialization, UART interaction, and bootloader compatibility.

## Features
- **UART Interaction**: Supports console commands (e.g., "help", "reset") via UART at 115200 baud, 8-N-1.
- **Simple Demo**: Toggles GPIO pins with a command-line interface via UART.
- **Reset Handling**: Implements `System_On_Bootloader_Reset()` from `bsp_system` for software reset to bootloader mode.

## Requirements
- **Hardware**:
  - STM32H745ZIT3 (OBC, dual-core).
  - STM32F765VGTx (EXP, single-core).
  - UART interface for console output.
- **Software**:
  - STM32CubeIDE for compilation and generating `.bin` files.
  - BSP library (`bsp_system`) for reset handling (assumed available in examples).

## Memory Map
Applications must be linked to the following addresses:
- **For STM32H745ZIT3 (OBC)**:
  - **Firmware 1 (Cortex-M7)**: `0x08040000` (Bank 1, Sectors 2-4, 384 KB).
  - **Firmware 2 (Cortex-M4)**: `0x08140000` (Bank 2, Sectors 2-4, 384 KB).
- **For STM32F765VGTx (EXP)**:
  - **Firmware**: `0x08020000` (Sectors 2-3, 256 KB).

For details, refer to [Memory Map](STM32_BOOTLOADER.md#memory-map).

The linker script must set the vector table to these addresses, and the `.bin` file must start at the specified address.

## Sample Application
The provided sample application includes:
- **For STM32H745ZIT3 (OBC)**:
  - **main_m7.c** (Cortex-M7):
    - Initializes system clock, GPIO, UART, and BSP library (`bsp_system`).
    - Runs a scheduler to handle tasks (e.g., processing UART commands).
    - Supports console commands via `command_init()` (assumed available).
    - Calls `System_On_Bootloader_Reset()` on software reset command.
  - **main_m4.c** (Cortex-M4):
    - Synchronizes with Cortex-M7 using HSEM.
    - Toggles a GPIO pin (PD7) every 100ms as a demo.
- **For STM32F765VGTx (EXP)**:
  - **main.c** (Cortex-M7):
    - Initializes system clock, GPIO, UART, and BSP library (`bsp_system`).
    - Supports console commands via `command_init()`.
    - Calls `System_On_Bootloader_Reset()` on software reset command.

### Key Code Snippets
#### Cortex-M7 for STM32H745ZIT3 (`main_m7.c`)
- Add macro `#define USE_CORE_M4`
- Remove code from `USER CODE BEGIN Boot_Mode_Sequence_0` to `USER CODE END Boot_Mode_Sequence_0` and from `USER CODE BEGIN Boot_Mode_Sequence_1` to `USER CODE END Boot_Mode_Sequence_1`
- Modify from `USER CODE BEGIN Boot_Mode_Sequence_2` to `USER CODE END Boot_Mode_Sequence_2` following this:
```c
#define USE_CORE_M4

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
/* USER CODE BEGIN Boot_Mode_Sequence_0 */

/* USER CODE END Boot_Mode_Sequence_0 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

/* USER CODE BEGIN Boot_Mode_Sequence_1 */
  
/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
/* USER CODE BEGIN Boot_Mode_Sequence_2 */
/*HW semaphore Clock enable*/
#ifdef USE_CORE_M4
  __HAL_RCC_HSEM_CLK_ENABLE();
  HAL_HSEM_FastTake(1);
  while (!HAL_HSEM_IsSemTaken(0));
  HAL_HSEM_Release(0, 0);
  while (!__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY));
  HAL_Init();
#endif
/* USER CODE END Boot_Mode_Sequence_2 */

  /* USER CODE BEGIN SysInit */
  ...........
```
- Initializes peripherals, BSP, and runs the scheduler.
- Handles software reset via `System_On_Bootloader_Reset()`.

#### Cortex-M4 for STM32H745ZIT3 (`main_m4.c`)
- Add macro `#define USE_CORE_M4` and modify from `USER CODE BEGIN Boot_Mode_Sequence_1` to `USER CODE END Boot_Mode_Sequence_1` following this:
```c
#define USE_CORE_M4

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

/* USER CODE BEGIN Boot_Mode_Sequence_1 */
#ifdef USE_CORE_M4
  __HAL_RCC_HSEM_CLK_ENABLE();
  HAL_HSEM_FastTake(0);
#else
  __HAL_RCC_HSEM_CLK_ENABLE();
  HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));
  HAL_PWREx_ClearPendingEvent();
  HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE, PWR_D2_DOMAIN);
  __HAL_HSEM_CLEAR_FLAG(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));
#endif

/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_MDMA_Init();
  MX_DMA_Init();
  MX_BDMA_Init();
  MX_GPIO_Init();
  MX_SDMMC2_MMC_Init();
  MX_USART6_UART_Init();
  MX_TIM2_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  Mgmt_HardwareSystemPreparing();

  Mgmt_SystemStart();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  ;;
	  // Should not go here
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
```

#### Cortex-M7 for STM32F765VGTx (`main.c`)
```c
#include "bsp_system.h"

int main(void)
{
  /* MCU Configuration */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  /* Initialize BSP */
  System_On_Bootloader_Reset(); // Handle reset to bootloader if triggered
  command_init();

  /* Infinite loop */
  while (1)
  {
    SchedulerRun();
  }
}
```
- Initializes BSP and handles software reset via `System_On_Bootloader_Reset()`.

### Developing a Compatible Application
To ensure the application works with the bootloader, follow these guidelines:

1. **Memory Layout**:
   - **For STM32H745ZIT3 (OBC)**:
     - **Cortex-M7 Linker Script** (STM32CubeIDE):
       ```ld
       MEMORY
       {
         RAM_D1 (xrw)   : ORIGIN = 0x24000000, LENGTH = 512K
         FLASH (rx)     : ORIGIN = 0x08040000, LENGTH = 384K
         DTCMRAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 128K
         RAM_D2 (xrw)   : ORIGIN = 0x30000000, LENGTH = 288K
         RAM_D3 (xrw)   : ORIGIN = 0x38000000, LENGTH = 64K
         ITCMRAM (xrw)  : ORIGIN = 0x00000000, LENGTH = 64K
         BKPRAM (xrw)   : ORIGIN = 0x38800000, LENGTH = 4K
       }
       SECTIONS
       {
         .bkpram (NOLOAD) :
         {
           *(.bkpram .bkpram.*)
           . = ALIGN(4);
         } > BKPRAM
       }
       ```
     - **Cortex-M4 Linker Script** (STM32CubeIDE):
       ```ld
       MEMORY
       {
         FLASH (rx)     : ORIGIN = 0x08140000, LENGTH = 384K
         RAM (xrw)      : ORIGIN = 0x10000000, LENGTH = 288K
       }
       ```
     - FLASH size reduced to 384 KB (3 sectors, 128 KB each).

   - **For STM32F765VGTx (EXP)**:
     - **Linker Script** (STM32CubeIDE):
       ```ld
       MEMORY
       {
         FLASH (rx)     : ORIGIN = 0x08020000, LENGTH = 256K
         RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 512K
         BKPRAM (xrw)   : ORIGIN = 0x40024000, LENGTH = 4K
       }
       SECTIONS
       {
         .bkpram (NOLOAD) :
         {
           *(.bkpram .bkpram.*)
           . = ALIGN(4);
         } > BKPRAM
       }
       ```
     - FLASH size set to 256 KB, starting at `0x08020000`.

2. **System Initialization**:
   - Initialize the system clock, peripherals, HAL, and BSP (`bsp_system`) in `main()`.
   - For STM32H745ZIT3 (OBC), Cortex-M4 must synchronize with Cortex-M7 using HSEM:
     ```c
     __HAL_RCC_HSEM_CLK_ENABLE();
     HAL_HSEM_FastTake(0);
     ```
   - For STM32H745ZIT3, enable dual-core support by defining:
     ```c
     #define USE_CORE_M4
     ```
   - For STM32H745ZIT3 Cortex-M4, after saving the `.ioc` file, modify `HAL_Init()` in `stm32h7xxx_hal.c` to set the ART base address:
     ```c
     #if defined(DUAL_CORE) && defined(CORE_CM4)
       __HAL_RCC_ART_CLK_ENABLE();
       __HAL_ART_CONFIG_BASE_ADDRESS(0x08140000UL);
       __HAL_ART_ENABLE();
     #endif
     ```

3. **UART Support**:
   - Configure UART to match the bootloader (115200 baud, 8-N-1).
   - Implement command handling, including "reset" to trigger `System_On_Bootloader_Reset()`.

4. **Reset Command**:
   - Implement a UART command (e.g., "reset") to trigger a software reset to bootloader mode:
     ```c
     if (strcmp(command, "reset") == 0) {
       System_On_Bootloader_Reset();
     }
     ```

5. **Binary Generation**:
   - Compile the application and generate a `.bin` file using STM32CubeIDE.
   - Ensure the `.bin` file starts at the correct address (`0x08040000` or `0x08140000` for OBC, `0x08020000` for EXP).

## Compilation and Flashing
1. **Create Project**:
   - Use STM32CubeIDE to create an application project.
   - Include `main_m7.c` and `main_m4.c` (for OBC) or `main.c` (for EXP).
2. **Configure Linker**:
   - Update the linker script to match the memory map.
3. **Build and Generate Binary**:
   - Compile the project and generate a `.bin` file.
4. **Flash via FOTA**:
   - Use the Python host script to upload the `.bin` file with associated metadata `.json`:
     ```bash
     python FOTA.py -mode seq -port /dev/ttyAMA3 -board OBC -bin1 app1.bin -meta1 meta1.json -bin2 app2.bin -meta2 meta2.json
     ```
     ```bash
     python FOTA.py -mode seq -port /dev/ttyAMA3 -board EXP -bin app.bin -meta meta.json
     ```

## Notes
- The application must not overwrite the bootloader or metadata regions.
- For OBC, M7 and M4 applications are synchronized via HSEM.
- For EXP, use the provided `bsp_system` as per the example.
- Test the application with the UART console to verify functionality.
- Metadata `.json` files contain SHA256 and other firmware details for verification.

## Limitations
- The sample application is minimal and may need expansion for real-world use.
- No support for secure boot or encrypted firmware.