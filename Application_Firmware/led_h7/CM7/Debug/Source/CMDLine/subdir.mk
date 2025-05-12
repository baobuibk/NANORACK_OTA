################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/CMDLine/cmdline.c \
../Source/CMDLine/command.c 

OBJS += \
./Source/CMDLine/cmdline.o \
./Source/CMDLine/command.o 

C_DEPS += \
./Source/CMDLine/cmdline.d \
./Source/CMDLine/command.d 


# Each subdirectory must supply rules for building sources it contributes
Source/CMDLine/%.o Source/CMDLine/%.su Source/CMDLine/%.cyclo: ../Source/CMDLine/%.c Source/CMDLine/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DCORE_CM7 -DUSE_HAL_DRIVER -DSTM32H745xx -DUSE_PWR_LDO_SUPPLY -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I"D:/WorkSpace/STM32 PRJ/led_h7/CM7/Source/UART" -I"D:/WorkSpace/STM32 PRJ/led_h7/CM7/Source/CMDLine" -I"D:/WorkSpace/STM32 PRJ/led_h7/CM7/Source/scheduler" -I"D:/WorkSpace/STM32 PRJ/led_h7/CM7/Source/bootmode" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Source-2f-CMDLine

clean-Source-2f-CMDLine:
	-$(RM) ./Source/CMDLine/cmdline.cyclo ./Source/CMDLine/cmdline.d ./Source/CMDLine/cmdline.o ./Source/CMDLine/cmdline.su ./Source/CMDLine/command.cyclo ./Source/CMDLine/command.d ./Source/CMDLine/command.o ./Source/CMDLine/command.su

.PHONY: clean-Source-2f-CMDLine

