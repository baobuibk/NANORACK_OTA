# STM32 Application for FOTA

## Overview
This document describes the requirements and structure for developing applications compatible with the STM32H745ZIT3 bootloader. A sample application is provided for both Cortex-M7 and Cortex-M4 cores, demonstrating how to initialize the system and interact with the bootloader.

## Features
- **UART Interaction**: Supports console commands (e.g., "help") via UART.
- **Simple Demo**: Toggles GPIO pins. Additionally, a command-line interface is provided via UART to demonstrate functionality.

## Requirements
- **Hardware**:
  - STM32H745ZIT3.
  - UART interface for console output.
- **Software**:
  - STM32CubeIDE for compilation and generating `.bin` files.

## Memory Map
Applications must be linked to the following addresses:
- **Firmware 1**: `0x08040000` (Bank 1, Sectors 2-5).
- **Firmware 2**: `0x08140000` (Bank 2, Sectors 2-5).

For detail, you can see this: [Memory Map](STM32_BOOTLOADER.md#memory-map)

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
#ifdef USE_CORE_M4
  __HAL_RCC_HSEM_CLK_ENABLE();
  HAL_HSEM_FastTake(1);

  while(!HAL_HSEM_IsSemTaken(0));
  HAL_HSEM_Release(0, 0);
  while (!__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY));
#endif
/* USER CODE END Boot_Mode_Sequence_2 */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  command_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  SchedulerRun();
  }
  /* USER CODE END 3 */
}
```
- Initializes peripherals and runs the scheduler.
- The scheduler handles UART command processing (via `command_init()`).

#### Cortex-M4 (`main_m4.c`)
```c
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
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_7);
	  HAL_Delay(100);

  }
  /* USER CODE END 3 */
}
```
- Synchronizes with M7 using HSEM.
- Toggles a GPIO pin as a simple demo.

## Developing a Compatible Application
To ensure the application works with the bootloader, follow these guidelines:

1. **Memory Layout**:
   - Set the linker script to place the vector table at `FIRMWARE_BANK1_BASE` (`0x08040000`) and `FIRMWARE_BANK2_BASE` (`0x08140000`).
   - Example linker script for core M7 (STM32CubeIDE):
      ```ld
      MEMORY
      {
        RAM_D1 (xrw)   : ORIGIN = 0x24000000, LENGTH =  512K
        FLASH  (rx)    : ORIGIN = 0x08040000, LENGTH = 512K
        DTCMRAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 128K
        RAM_D2 (xrw)   : ORIGIN = 0x30000000, LENGTH = 288K
        RAM_D3 (xrw)   : ORIGIN = 0x38000000, LENGTH = 64K
        ITCMRAM (xrw)  : ORIGIN = 0x00000000, LENGTH = 64K
      }
      ```
      Modify the linker script to configure the address and size of the FLASH memory. The origin address is set to `FIRMWARE_BANK1_BASE`, spanning 4 sectors (sectors 2 to 5), with a total capacity of 512KB (as per the datasheet: 128KB x 4).

   - Example linker script for core M4 (STM32CubeIDE):
     ```ld
      MEMORY
      {
        FLASH (rx)     : ORIGIN = 0x08140000, LENGTH = 512K
        RAM (xrw)      : ORIGIN = 0x10000000, LENGTH = 288K
      }
     ```
     For the Cortex-M4 core, similar to the Cortex-M7 core, the linker script must be modified to configure the address and size of the FLASH memory. The origin address is set to `FIRMWARE_BANK2_BASE`, spanning 4 sectors, with a total capacity of 512KB (128KB x 4).

2. **System Initialization**:
   - Initialize the system clock, peripherals, and HAL in `main()`.
   - For STM32H745ZIT3, the Cortex-M4 must synchronize with Cortex-M7 using HSEM:
     ```c
     __HAL_RCC_HSEM_CLK_ENABLE();
     HAL_HSEM_FastTake(0); 
     ```
     According to the sample application, to use both cores, you only need to add the following code:
     ```c
     #define USE_CORE_M4
     ```

3. **UART Support**:
   - If the application uses UART (e.g., for console), configure it to match the bootloader (115200 baud, 8-N-1).
   - Implement command handling (e.g., "reset" to return to bootloader mode) or use hardware and sortware reset to boot to bootloader mode.

5. **Reset Command**:
   - To return to bootloader mode, implement a UART command (e.g., "reset"):
     ```c
     if (strcmp(command, "reset") == 0) {
       NVIC_SystemReset();
     }
     ```
   - The bootloader will handle the reset and enter bootloader mode if a connection is detected.

6. **Binary Generation**:
   - Compile the application and generate a `.bin` file using STM32CubeIDE.
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
     python FOTA.py -mode seq -port COM5 -mcu H7 -bin1 app1.bin -v1 1.0.0 -bin2 app2.bin -v2 1.0.0
     ```

## Notes
- The application must not overwrite the bootloader or metadata regions.
- Both M7 and M4 applications are synchronized via HSEM.
- Test the application with the UART console to verify functionality.

## Limitations
- The sample application is minimal and may need expansion for real-world use.
- No support for secure boot or encrypted firmware.
