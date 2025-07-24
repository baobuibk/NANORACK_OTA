################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
D:/WorkSpace/STM32\ PRJ/04_obc_v111/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c \
D:/WorkSpace/STM32\ PRJ/04_obc_v111/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c \
D:/WorkSpace/STM32\ PRJ/04_obc_v111/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
D:/WorkSpace/STM32\ PRJ/04_obc_v111/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c 

OBJS += \
./Middlewares/ST/STM32_USB_Device_Library/usbd_cdc.o \
./Middlewares/ST/STM32_USB_Device_Library/usbd_core.o \
./Middlewares/ST/STM32_USB_Device_Library/usbd_ctlreq.o \
./Middlewares/ST/STM32_USB_Device_Library/usbd_ioreq.o 

C_DEPS += \
./Middlewares/ST/STM32_USB_Device_Library/usbd_cdc.d \
./Middlewares/ST/STM32_USB_Device_Library/usbd_core.d \
./Middlewares/ST/STM32_USB_Device_Library/usbd_ctlreq.d \
./Middlewares/ST/STM32_USB_Device_Library/usbd_ioreq.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/ST/STM32_USB_Device_Library/usbd_cdc.o: D:/WorkSpace/STM32\ PRJ/04_obc_v111/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c Middlewares/ST/STM32_USB_Device_Library/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32H745xx -DUSE_PWR_LDO_SUPPLY -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/_Target4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M0_App4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M1_Driver4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M4_Util4" -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Middlewares/ST/STM32_USB_Device_Library/usbd_cdc.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/ST/STM32_USB_Device_Library/usbd_core.o: D:/WorkSpace/STM32\ PRJ/04_obc_v111/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c Middlewares/ST/STM32_USB_Device_Library/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32H745xx -DUSE_PWR_LDO_SUPPLY -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/_Target4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M0_App4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M1_Driver4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M4_Util4" -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Middlewares/ST/STM32_USB_Device_Library/usbd_core.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/ST/STM32_USB_Device_Library/usbd_ctlreq.o: D:/WorkSpace/STM32\ PRJ/04_obc_v111/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c Middlewares/ST/STM32_USB_Device_Library/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32H745xx -DUSE_PWR_LDO_SUPPLY -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/_Target4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M0_App4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M1_Driver4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M4_Util4" -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Middlewares/ST/STM32_USB_Device_Library/usbd_ctlreq.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/ST/STM32_USB_Device_Library/usbd_ioreq.o: D:/WorkSpace/STM32\ PRJ/04_obc_v111/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c Middlewares/ST/STM32_USB_Device_Library/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32H745xx -DUSE_PWR_LDO_SUPPLY -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/_Target4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M0_App4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M1_Driver4" -I"D:/WorkSpace/STM32 PRJ/04_obc_v111/CM4/0_DEV/M4_Util4" -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Middlewares/ST/STM32_USB_Device_Library/usbd_ioreq.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-ST-2f-STM32_USB_Device_Library

clean-Middlewares-2f-ST-2f-STM32_USB_Device_Library:
	-$(RM) ./Middlewares/ST/STM32_USB_Device_Library/usbd_cdc.cyclo ./Middlewares/ST/STM32_USB_Device_Library/usbd_cdc.d ./Middlewares/ST/STM32_USB_Device_Library/usbd_cdc.o ./Middlewares/ST/STM32_USB_Device_Library/usbd_cdc.su ./Middlewares/ST/STM32_USB_Device_Library/usbd_core.cyclo ./Middlewares/ST/STM32_USB_Device_Library/usbd_core.d ./Middlewares/ST/STM32_USB_Device_Library/usbd_core.o ./Middlewares/ST/STM32_USB_Device_Library/usbd_core.su ./Middlewares/ST/STM32_USB_Device_Library/usbd_ctlreq.cyclo ./Middlewares/ST/STM32_USB_Device_Library/usbd_ctlreq.d ./Middlewares/ST/STM32_USB_Device_Library/usbd_ctlreq.o ./Middlewares/ST/STM32_USB_Device_Library/usbd_ctlreq.su ./Middlewares/ST/STM32_USB_Device_Library/usbd_ioreq.cyclo ./Middlewares/ST/STM32_USB_Device_Library/usbd_ioreq.d ./Middlewares/ST/STM32_USB_Device_Library/usbd_ioreq.o ./Middlewares/ST/STM32_USB_Device_Library/usbd_ioreq.su

.PHONY: clean-Middlewares-2f-ST-2f-STM32_USB_Device_Library

