################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/bootmode/bootmode.c 

OBJS += \
./Source/bootmode/bootmode.o 

C_DEPS += \
./Source/bootmode/bootmode.d 


# Each subdirectory must supply rules for building sources it contributes
Source/bootmode/%.o Source/bootmode/%.su Source/bootmode/%.cyclo: ../Source/bootmode/%.c Source/bootmode/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DCORE_CM7 -DUSE_HAL_DRIVER -DSTM32H745xx -DUSE_PWR_LDO_SUPPLY -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I"D:/WorkSpace/STM32 PRJ/led_h7/CM7/Source/UART" -I"D:/WorkSpace/STM32 PRJ/led_h7/CM7/Source/CMDLine" -I"D:/WorkSpace/STM32 PRJ/led_h7/CM7/Source/scheduler" -I"D:/WorkSpace/STM32 PRJ/led_h7/CM7/Source/bootmode" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Source-2f-bootmode

clean-Source-2f-bootmode:
	-$(RM) ./Source/bootmode/bootmode.cyclo ./Source/bootmode/bootmode.d ./Source/bootmode/bootmode.o ./Source/bootmode/bootmode.su

.PHONY: clean-Source-2f-bootmode

