################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../1_DEV/M4_Utils/zMode_RB/zmode_rb.c 

C_DEPS += \
./1_DEV/M4_Utils/zMode_RB/zmode_rb.d 

OBJS += \
./1_DEV/M4_Utils/zMode_RB/zmode_rb.o 


# Each subdirectory must supply rules for building sources it contributes
1_DEV/M4_Utils/zMode_RB/%.o 1_DEV/M4_Utils/zMode_RB/%.su 1_DEV/M4_Utils/zMode_RB/%.cyclo: ../1_DEV/M4_Utils/zMode_RB/%.c 1_DEV/M4_Utils/zMode_RB/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DCORE_CM7 -DUSE_HAL_DRIVER -DSTM32H745xx -DUSE_PWR_LDO_SUPPLY -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M0_App/Mgmt" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M0_App/NoOS" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M0_App/OS" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M0_App/Structs" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M0_App/Tasks" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M0_App" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M1_Drivers/I2C" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M1_Drivers/IPC" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M1_Drivers/SDMMC" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M1_Drivers/SPI" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M1_Drivers/UART" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M1_Drivers/UART_DMA" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M1_Drivers/USB_CDC" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M5_ThirdParty/FreeRTOS/FreeRTOS_Config" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M5_ThirdParty/FreeRTOS/FreeRTOS-Kernel/include" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M5_ThirdParty/FreeRTOS/FreeRTOS-Kernel/portable/GCC/ARM_CM7/r0p1" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M5_ThirdParty/FreeRTOS/FreeRTOS-Kernel/portable/MemMang" -I"D:/LAB_PROJECT_7.7.7.7.7.7.7/stm32cubeide/04_obc_v111/CM7/1_DEV/M0_App" -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-1_DEV-2f-M4_Utils-2f-zMode_RB

clean-1_DEV-2f-M4_Utils-2f-zMode_RB:
	-$(RM) ./1_DEV/M4_Utils/zMode_RB/zmode_rb.cyclo ./1_DEV/M4_Utils/zMode_RB/zmode_rb.d ./1_DEV/M4_Utils/zMode_RB/zmode_rb.o ./1_DEV/M4_Utils/zMode_RB/zmode_rb.su

.PHONY: clean-1_DEV-2f-M4_Utils-2f-zMode_RB

