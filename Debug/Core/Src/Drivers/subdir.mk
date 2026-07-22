################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Drivers/cc1101.c \
../Core/Src/Drivers/gy273.c \
../Core/Src/Drivers/gy6500.c \
../Core/Src/Drivers/i2c_helper.c \
../Core/Src/Drivers/pca9685.c 

OBJS += \
./Core/Src/Drivers/cc1101.o \
./Core/Src/Drivers/gy273.o \
./Core/Src/Drivers/gy6500.o \
./Core/Src/Drivers/i2c_helper.o \
./Core/Src/Drivers/pca9685.o 

C_DEPS += \
./Core/Src/Drivers/cc1101.d \
./Core/Src/Drivers/gy273.d \
./Core/Src/Drivers/gy6500.d \
./Core/Src/Drivers/i2c_helper.d \
./Core/Src/Drivers/pca9685.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Drivers/%.o Core/Src/Drivers/%.su Core/Src/Drivers/%.cyclo: ../Core/Src/Drivers/%.c Core/Src/Drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Drivers

clean-Core-2f-Src-2f-Drivers:
	-$(RM) ./Core/Src/Drivers/cc1101.cyclo ./Core/Src/Drivers/cc1101.d ./Core/Src/Drivers/cc1101.o ./Core/Src/Drivers/cc1101.su ./Core/Src/Drivers/gy273.cyclo ./Core/Src/Drivers/gy273.d ./Core/Src/Drivers/gy273.o ./Core/Src/Drivers/gy273.su ./Core/Src/Drivers/gy6500.cyclo ./Core/Src/Drivers/gy6500.d ./Core/Src/Drivers/gy6500.o ./Core/Src/Drivers/gy6500.su ./Core/Src/Drivers/i2c_helper.cyclo ./Core/Src/Drivers/i2c_helper.d ./Core/Src/Drivers/i2c_helper.o ./Core/Src/Drivers/i2c_helper.su ./Core/Src/Drivers/pca9685.cyclo ./Core/Src/Drivers/pca9685.d ./Core/Src/Drivers/pca9685.o ./Core/Src/Drivers/pca9685.su

.PHONY: clean-Core-2f-Src-2f-Drivers

