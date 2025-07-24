################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32H7xx_HAL_Driver/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include Common/Src/subdir.mk
-include 2_OUTPUT/elf/subdir.mk
-include 1_DEV/M5_ThirdParty/ModFSP/subdir.mk
-include 1_DEV/M5_ThirdParty/MIN_R01/min_app/subdir.mk
-include 1_DEV/M5_ThirdParty/MIN_R01/min/subdir.mk
-include 1_DEV/M5_ThirdParty/LWL/subdir.mk
-include 1_DEV/M5_ThirdParty/FreeRTOS/FreeRTOS-Kernel/portable/MemMang/subdir.mk
-include 1_DEV/M5_ThirdParty/FreeRTOS/FreeRTOS-Kernel/portable/GCC/ARM_CM7/r0p1/subdir.mk
-include 1_DEV/M5_ThirdParty/FreeRTOS/FreeRTOS-Kernel/subdir.mk
-include 1_DEV/M5_ThirdParty/FatFS_R015/Option/subdir.mk
-include 1_DEV/M5_ThirdParty/FatFS_R015/Drivers/subdir.mk
-include 1_DEV/M5_ThirdParty/FatFS_R015/Core/subdir.mk
-include 1_DEV/M5_ThirdParty/BinaryScript/storage/subdir.mk
-include 1_DEV/M5_ThirdParty/BinaryScript/runner/subdir.mk
-include 1_DEV/M5_ThirdParty/BinaryScript/port/subdir.mk
-include 1_DEV/M5_ThirdParty/BinaryScript/parser/subdir.mk
-include 1_DEV/M5_ThirdParty/BinaryScript/logger/subdir.mk
-include 1_DEV/M4_Utils/Tick/subdir.mk
-include 1_DEV/M4_Utils/RingBuffer/subdir.mk
-include 1_DEV/M4_Utils/DateTime/subdir.mk
-include 1_DEV/M3_Devices/SPI_SlaveOfCM4/subdir.mk
-include 1_DEV/M3_Devices/SPI_MasterOfEXP/subdir.mk
-include 1_DEV/M3_Devices/SPI_FRAM/subdir.mk
-include 1_DEV/M3_Devices/IO_ExWD-TPL5010/subdir.mk
-include 1_DEV/M3_Devices/I2C_RTC-RV3129/subdir.mk
-include 1_DEV/M2_System/bsp_system/subdir.mk
-include 1_DEV/M2_System/SystemManager/subdir.mk
-include 1_DEV/M2_System/SysLog_Queue/subdir.mk
-include 1_DEV/M2_System/SysLog/subdir.mk
-include 1_DEV/M2_System/ScriptManager/subdir.mk
-include 1_DEV/M2_System/MIN_Process/subdir.mk
-include 1_DEV/M2_System/FileSystem/subdir.mk
-include 1_DEV/M2_System/Dmesg/subdir.mk
-include 1_DEV/M2_System/CLI_Terminal/CLI_Src/subdir.mk
-include 1_DEV/M2_System/CLI_Terminal/CLI_Setup/subdir.mk
-include 1_DEV/M2_System/CLI_Terminal/CLI_Command/subdir.mk
-include 1_DEV/M2_System/CLI_Terminal/CLI_Auth/subdir.mk
-include 1_DEV/M2_System/AliveCM4/subdir.mk
-include 1_DEV/M1_Drivers/UART_DMA/subdir.mk
-include 1_DEV/M1_Drivers/UART/subdir.mk
-include 1_DEV/M1_Drivers/Shared_REG/subdir.mk
-include 1_DEV/M1_Drivers/SPI/subdir.mk
-include 1_DEV/M1_Drivers/SDMMC/subdir.mk
-include 1_DEV/M1_Drivers/IPC/subdir.mk
-include 1_DEV/M1_Drivers/I2C/subdir.mk
-include 1_DEV/M0_App/Tasks/subdir.mk
-include 1_DEV/M0_App/Structs/subdir.mk
-include 1_DEV/M0_App/OS/subdir.mk
-include 1_DEV/M0_App/Mgmt/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := 04_obc_v111_CM7
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
04_obc_v111_CM7.elf \

MAP_FILES += \
04_obc_v111_CM7.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
04_obc_v111_CM7.list \

OBJCOPY_HEX += \
04_obc_v111_CM7.hex \

OBJCOPY_BIN += \
04_obc_v111_CM7.bin \

OBJCOPY_VERILOG += \
04_obc_v111_CM7.verilog \


# All Target
all:
	+@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: 04_obc_v111_CM7.elf secondary-outputs

# Tool invocations
04_obc_v111_CM7.elf 04_obc_v111_CM7.map: $(OBJS) $(USER_OBJS) D:\WorkSpace\STM32\ PRJ\04_obc_v111\CM7\STM32H745ZITX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "04_obc_v111_CM7.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m7 -T"D:\WorkSpace\STM32 PRJ\04_obc_v111\CM7\STM32H745ZITX_FLASH.ld" --specs=nosys.specs -Wl,-Map="04_obc_v111_CM7.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

04_obc_v111_CM7.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "04_obc_v111_CM7.list"
	@echo 'Finished building: $@'
	@echo ' '

04_obc_v111_CM7.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "04_obc_v111_CM7.hex"
	@echo 'Finished building: $@'
	@echo ' '

04_obc_v111_CM7.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "04_obc_v111_CM7.bin"
	@echo 'Finished building: $@'
	@echo ' '

04_obc_v111_CM7.verilog: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O verilog $(EXECUTABLES) "04_obc_v111_CM7.verilog"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) 04_obc_v111_CM7.bin 04_obc_v111_CM7.elf 04_obc_v111_CM7.hex 04_obc_v111_CM7.list 04_obc_v111_CM7.map 04_obc_v111_CM7.verilog default.size.stdout
	-@echo ' '

post-build:
	mkdir -p ../2_OUTPUT/bin ../2_OUTPUT/elf ../2_OUTPUT/hex "C:/ST/STM32CubeIDE_1.15.1/STM32CubeIDE/fw/bin"
	rm -f ../2_OUTPUT/bin/* ../2_OUTPUT/elf/* ../2_OUTPUT/hex/* C:/ST/STM32CubeIDE_1.15.1/STM32CubeIDE/fw/bin/*.bin
	cp "04_obc_v111_CM7.bin" "../2_OUTPUT/bin/04_obc_v111_CM7_output.bin"
	cp "04_obc_v111_CM7.bin" "C:/ST/STM32CubeIDE_1.15.1/STM32CubeIDE/fw/bin/04_obc_v111_CM7.bin"
	cp "04_obc_v111_CM7.elf" "../2_OUTPUT/elf/04_obc_v111_CM7_output.elf"
	cp "04_obc_v111_CM7.hex" "../2_OUTPUT/hex/04_obc_v111_CM7_output.hex"
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN) $(OBJCOPY_VERILOG)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified post-build

-include ../makefile.targets
