################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Drivers/DX-LR30_Driver/UserConfig.c \
../Core/Src/Drivers/DX-LR30_Driver/driver_DIO1.c \
../Core/Src/Drivers/DX-LR30_Driver/lr_fhss_mac.c \
../Core/Src/Drivers/DX-LR30_Driver/myqueue.c \
../Core/Src/Drivers/DX-LR30_Driver/sx126x.c \
../Core/Src/Drivers/DX-LR30_Driver/sx126x_driver_version.c \
../Core/Src/Drivers/DX-LR30_Driver/sx126x_hal.c \
../Core/Src/Drivers/DX-LR30_Driver/sx126x_lr_fhss.c 

OBJS += \
./Core/Src/Drivers/DX-LR30_Driver/UserConfig.o \
./Core/Src/Drivers/DX-LR30_Driver/driver_DIO1.o \
./Core/Src/Drivers/DX-LR30_Driver/lr_fhss_mac.o \
./Core/Src/Drivers/DX-LR30_Driver/myqueue.o \
./Core/Src/Drivers/DX-LR30_Driver/sx126x.o \
./Core/Src/Drivers/DX-LR30_Driver/sx126x_driver_version.o \
./Core/Src/Drivers/DX-LR30_Driver/sx126x_hal.o \
./Core/Src/Drivers/DX-LR30_Driver/sx126x_lr_fhss.o 

C_DEPS += \
./Core/Src/Drivers/DX-LR30_Driver/UserConfig.d \
./Core/Src/Drivers/DX-LR30_Driver/driver_DIO1.d \
./Core/Src/Drivers/DX-LR30_Driver/lr_fhss_mac.d \
./Core/Src/Drivers/DX-LR30_Driver/myqueue.d \
./Core/Src/Drivers/DX-LR30_Driver/sx126x.d \
./Core/Src/Drivers/DX-LR30_Driver/sx126x_driver_version.d \
./Core/Src/Drivers/DX-LR30_Driver/sx126x_hal.d \
./Core/Src/Drivers/DX-LR30_Driver/sx126x_lr_fhss.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Drivers/DX-LR30_Driver/%.o Core/Src/Drivers/DX-LR30_Driver/%.su Core/Src/Drivers/DX-LR30_Driver/%.cyclo: ../Core/Src/Drivers/DX-LR30_Driver/%.c Core/Src/Drivers/DX-LR30_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Drivers-2f-DX-2d-LR30_Driver

clean-Core-2f-Src-2f-Drivers-2f-DX-2d-LR30_Driver:
	-$(RM) ./Core/Src/Drivers/DX-LR30_Driver/UserConfig.cyclo ./Core/Src/Drivers/DX-LR30_Driver/UserConfig.d ./Core/Src/Drivers/DX-LR30_Driver/UserConfig.o ./Core/Src/Drivers/DX-LR30_Driver/UserConfig.su ./Core/Src/Drivers/DX-LR30_Driver/driver_DIO1.cyclo ./Core/Src/Drivers/DX-LR30_Driver/driver_DIO1.d ./Core/Src/Drivers/DX-LR30_Driver/driver_DIO1.o ./Core/Src/Drivers/DX-LR30_Driver/driver_DIO1.su ./Core/Src/Drivers/DX-LR30_Driver/lr_fhss_mac.cyclo ./Core/Src/Drivers/DX-LR30_Driver/lr_fhss_mac.d ./Core/Src/Drivers/DX-LR30_Driver/lr_fhss_mac.o ./Core/Src/Drivers/DX-LR30_Driver/lr_fhss_mac.su ./Core/Src/Drivers/DX-LR30_Driver/myqueue.cyclo ./Core/Src/Drivers/DX-LR30_Driver/myqueue.d ./Core/Src/Drivers/DX-LR30_Driver/myqueue.o ./Core/Src/Drivers/DX-LR30_Driver/myqueue.su ./Core/Src/Drivers/DX-LR30_Driver/sx126x.cyclo ./Core/Src/Drivers/DX-LR30_Driver/sx126x.d ./Core/Src/Drivers/DX-LR30_Driver/sx126x.o ./Core/Src/Drivers/DX-LR30_Driver/sx126x.su ./Core/Src/Drivers/DX-LR30_Driver/sx126x_driver_version.cyclo ./Core/Src/Drivers/DX-LR30_Driver/sx126x_driver_version.d ./Core/Src/Drivers/DX-LR30_Driver/sx126x_driver_version.o ./Core/Src/Drivers/DX-LR30_Driver/sx126x_driver_version.su ./Core/Src/Drivers/DX-LR30_Driver/sx126x_hal.cyclo ./Core/Src/Drivers/DX-LR30_Driver/sx126x_hal.d ./Core/Src/Drivers/DX-LR30_Driver/sx126x_hal.o ./Core/Src/Drivers/DX-LR30_Driver/sx126x_hal.su ./Core/Src/Drivers/DX-LR30_Driver/sx126x_lr_fhss.cyclo ./Core/Src/Drivers/DX-LR30_Driver/sx126x_lr_fhss.d ./Core/Src/Drivers/DX-LR30_Driver/sx126x_lr_fhss.o ./Core/Src/Drivers/DX-LR30_Driver/sx126x_lr_fhss.su

.PHONY: clean-Core-2f-Src-2f-Drivers-2f-DX-2d-LR30_Driver

