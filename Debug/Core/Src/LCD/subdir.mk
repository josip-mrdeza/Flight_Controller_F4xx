################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/LCD/lcd1602_i2c.c \
../Core/Src/LCD/menu_helper.c \
../Core/Src/LCD/ssd1315.c 

OBJS += \
./Core/Src/LCD/lcd1602_i2c.o \
./Core/Src/LCD/menu_helper.o \
./Core/Src/LCD/ssd1315.o 

C_DEPS += \
./Core/Src/LCD/lcd1602_i2c.d \
./Core/Src/LCD/menu_helper.d \
./Core/Src/LCD/ssd1315.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/LCD/%.o Core/Src/LCD/%.su Core/Src/LCD/%.cyclo: ../Core/Src/LCD/%.c Core/Src/LCD/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-LCD

clean-Core-2f-Src-2f-LCD:
	-$(RM) ./Core/Src/LCD/lcd1602_i2c.cyclo ./Core/Src/LCD/lcd1602_i2c.d ./Core/Src/LCD/lcd1602_i2c.o ./Core/Src/LCD/lcd1602_i2c.su ./Core/Src/LCD/menu_helper.cyclo ./Core/Src/LCD/menu_helper.d ./Core/Src/LCD/menu_helper.o ./Core/Src/LCD/menu_helper.su ./Core/Src/LCD/ssd1315.cyclo ./Core/Src/LCD/ssd1315.d ./Core/Src/LCD/ssd1315.o ./Core/Src/LCD/ssd1315.su

.PHONY: clean-Core-2f-Src-2f-LCD

