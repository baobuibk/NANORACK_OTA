################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../0_DEV/M1_Driver4/UART_DMA/uart_dma_driver.c 

OBJS += \
./0_DEV/M1_Driver4/UART_DMA/uart_dma_driver.o 

C_DEPS += \
./0_DEV/M1_Driver4/UART_DMA/uart_dma_driver.d 


# Each subdirectory must supply rules for building sources it contributes
0_DEV/M1_Driver4/UART_DMA/%.o 0_DEV/M1_Driver4/UART_DMA/%.su 0_DEV/M1_Driver4/UART_DMA/%.cyclo: ../0_DEV/M1_Driver4/UART_DMA/%.c 0_DEV/M1_Driver4/UART_DMA/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32H745xx -DUSE_PWR_LDO_SUPPLY -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/_Target4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M0_App4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M1_Driver4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M4_Util4" -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-0_DEV-2f-M1_Driver4-2f-UART_DMA

clean-0_DEV-2f-M1_Driver4-2f-UART_DMA:
	-$(RM) ./0_DEV/M1_Driver4/UART_DMA/uart_dma_driver.cyclo ./0_DEV/M1_Driver4/UART_DMA/uart_dma_driver.d ./0_DEV/M1_Driver4/UART_DMA/uart_dma_driver.o ./0_DEV/M1_Driver4/UART_DMA/uart_dma_driver.su

.PHONY: clean-0_DEV-2f-M1_Driver4-2f-UART_DMA

