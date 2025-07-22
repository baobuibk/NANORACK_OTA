################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/UART/RingBuffer.c \
../Source/UART/UART.c 

OBJS += \
./Source/UART/RingBuffer.o \
./Source/UART/UART.o 

C_DEPS += \
./Source/UART/RingBuffer.d \
./Source/UART/UART.d 


# Each subdirectory must supply rules for building sources it contributes
Source/UART/%.o Source/UART/%.su Source/UART/%.cyclo: ../Source/UART/%.c Source/UART/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F765xx -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"D:/WorkSpace/STM32 PRJ/EXP_FOTA/Source/bootloader" -I"D:/WorkSpace/STM32 PRJ/EXP_FOTA/Source/scheduler" -I"D:/WorkSpace/STM32 PRJ/EXP_FOTA/Source/UART" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Source-2f-UART

clean-Source-2f-UART:
	-$(RM) ./Source/UART/RingBuffer.cyclo ./Source/UART/RingBuffer.d ./Source/UART/RingBuffer.o ./Source/UART/RingBuffer.su ./Source/UART/UART.cyclo ./Source/UART/UART.d ./Source/UART/UART.o ./Source/UART/UART.su

.PHONY: clean-Source-2f-UART

