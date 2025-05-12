################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/scheduler/scheduler.c 

OBJS += \
./Source/scheduler/scheduler.o 

C_DEPS += \
./Source/scheduler/scheduler.d 


# Each subdirectory must supply rules for building sources it contributes
Source/scheduler/%.o Source/scheduler/%.su Source/scheduler/%.cyclo: ../Source/scheduler/%.c Source/scheduler/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DCORE_CM7 -DUSE_HAL_DRIVER -DSTM32H745xx -DUSE_FULL_LL_DRIVER -DUSE_PWR_LDO_SUPPLY -c -I../Core/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I"D:/WorkSpace/STM32 PRJ/OBC_FOTA/CM7/Source/UART" -I"D:/WorkSpace/STM32 PRJ/OBC_FOTA/CM7/Source/bootloader" -I"D:/WorkSpace/STM32 PRJ/OBC_FOTA/CM7/Source/Flash" -I"D:/WorkSpace/STM32 PRJ/OBC_FOTA/CM7/Source/scheduler" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Source-2f-scheduler

clean-Source-2f-scheduler:
	-$(RM) ./Source/scheduler/scheduler.cyclo ./Source/scheduler/scheduler.d ./Source/scheduler/scheduler.o ./Source/scheduler/scheduler.su

.PHONY: clean-Source-2f-scheduler

