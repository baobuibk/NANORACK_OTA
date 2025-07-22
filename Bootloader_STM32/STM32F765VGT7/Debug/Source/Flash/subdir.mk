################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/Flash/flash.c 

OBJS += \
./Source/Flash/flash.o 

C_DEPS += \
./Source/Flash/flash.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Flash/%.o Source/Flash/%.su Source/Flash/%.cyclo: ../Source/Flash/%.c Source/Flash/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F765xx -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"D:/WorkSpace/STM32 PRJ/EXP_FOTA/Source/bootloader" -I"D:/WorkSpace/STM32 PRJ/EXP_FOTA/Source/Flash" -I"D:/WorkSpace/STM32 PRJ/EXP_FOTA/Source/scheduler" -I"D:/WorkSpace/STM32 PRJ/EXP_FOTA/Source/UART" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Source-2f-Flash

clean-Source-2f-Flash:
	-$(RM) ./Source/Flash/flash.cyclo ./Source/Flash/flash.d ./Source/Flash/flash.o ./Source/Flash/flash.su

.PHONY: clean-Source-2f-Flash

