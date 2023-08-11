################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/bgapi.c \
../Src/crc32.c \
../Src/flash.c \
../Src/main.c \
../Src/ota.c \
../Src/stm32u5xx_hal_msp.c \
../Src/stm32u5xx_it.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/system_stm32u5xx.c \
../Src/temp.c \
../Src/uart.c \
../Src/util.c \
../Src/validate.c 

OBJS += \
./Src/bgapi.o \
./Src/crc32.o \
./Src/flash.o \
./Src/main.o \
./Src/ota.o \
./Src/stm32u5xx_hal_msp.o \
./Src/stm32u5xx_it.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/system_stm32u5xx.o \
./Src/temp.o \
./Src/uart.o \
./Src/util.o \
./Src/validate.o 

C_DEPS += \
./Src/bgapi.d \
./Src/crc32.d \
./Src/flash.d \
./Src/main.d \
./Src/ota.d \
./Src/stm32u5xx_hal_msp.d \
./Src/stm32u5xx_it.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/system_stm32u5xx.d \
./Src/temp.d \
./Src/uart.d \
./Src/util.d \
./Src/validate.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32U5A5xx -c -I../Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/bgapi.cyclo ./Src/bgapi.d ./Src/bgapi.o ./Src/bgapi.su ./Src/crc32.cyclo ./Src/crc32.d ./Src/crc32.o ./Src/crc32.su ./Src/flash.cyclo ./Src/flash.d ./Src/flash.o ./Src/flash.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/ota.cyclo ./Src/ota.d ./Src/ota.o ./Src/ota.su ./Src/stm32u5xx_hal_msp.cyclo ./Src/stm32u5xx_hal_msp.d ./Src/stm32u5xx_hal_msp.o ./Src/stm32u5xx_hal_msp.su ./Src/stm32u5xx_it.cyclo ./Src/stm32u5xx_it.d ./Src/stm32u5xx_it.o ./Src/stm32u5xx_it.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su ./Src/system_stm32u5xx.cyclo ./Src/system_stm32u5xx.d ./Src/system_stm32u5xx.o ./Src/system_stm32u5xx.su ./Src/temp.cyclo ./Src/temp.d ./Src/temp.o ./Src/temp.su ./Src/uart.cyclo ./Src/uart.d ./Src/uart.o ./Src/uart.su ./Src/util.cyclo ./Src/util.d ./Src/util.o ./Src/util.su ./Src/validate.cyclo ./Src/validate.d ./Src/validate.o ./Src/validate.su

.PHONY: clean-Src

