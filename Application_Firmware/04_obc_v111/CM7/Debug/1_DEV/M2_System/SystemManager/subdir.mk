################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../1_DEV/M2_System/SystemManager/sys_manager.c 

C_DEPS += \
./1_DEV/M2_System/SystemManager/sys_manager.d 

OBJS += \
./1_DEV/M2_System/SystemManager/sys_manager.o 


# Each subdirectory must supply rules for building sources it contributes
1_DEV/M2_System/SystemManager/%.o 1_DEV/M2_System/SystemManager/%.su 1_DEV/M2_System/SystemManager/%.cyclo: ../1_DEV/M2_System/SystemManager/%.c 1_DEV/M2_System/SystemManager/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DCORE_CM7 -DUSE_HAL_DRIVER -DSTM32H745xx -DUSE_PWR_LDO_SUPPLY -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M5_ThirdParty/LWL" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M5_ThirdParty/BinaryScript" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M5_ThirdParty/ModFSP" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M1_Drivers/Shared_REG" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M5_ThirdParty/MIN_R01" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M2_System/FileSystem" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M2_System" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M5_ThirdParty/FatFS_R015/Core" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M5_ThirdParty/FatFS_R015/Drivers" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M5_ThirdParty/FatFS_R015/Option" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M2_System" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M3_Devices" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/_Target" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M4_Utils" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M0_App/Mgmt" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M0_App/OS" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M0_App/Structs" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M0_App/Tasks" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M0_App" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M1_Drivers/I2C" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M1_Drivers/IPC" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M1_Drivers/SDMMC" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M1_Drivers/SPI" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M1_Drivers/UART" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M1_Drivers/UART_DMA" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M5_ThirdParty/FreeRTOS/FreeRTOS_Config" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M5_ThirdParty/FreeRTOS/FreeRTOS-Kernel/include" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M5_ThirdParty/FreeRTOS/FreeRTOS-Kernel/portable/GCC/ARM_CM7/r0p1" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M5_ThirdParty/FreeRTOS/FreeRTOS-Kernel/portable/MemMang" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M0_App" -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM7/1_DEV/M2_System/bsp_system" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-1_DEV-2f-M2_System-2f-SystemManager

clean-1_DEV-2f-M2_System-2f-SystemManager:
	-$(RM) ./1_DEV/M2_System/SystemManager/sys_manager.cyclo ./1_DEV/M2_System/SystemManager/sys_manager.d ./1_DEV/M2_System/SystemManager/sys_manager.o ./1_DEV/M2_System/SystemManager/sys_manager.su

.PHONY: clean-1_DEV-2f-M2_System-2f-SystemManager

