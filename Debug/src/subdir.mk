################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/delay.c \
../src/main.c \
../src/motion.c \
../src/sensor.c \
../src/stm32f4xx_it.c \
../src/syscalls.c \
../src/system_stm32f4xx.c \
../src/tm_stm32f4_exti.c \
../src/tm_stm32f4_gpio.c \
../src/usart.c \
../src/utils.c 

OBJS += \
./src/delay.o \
./src/main.o \
./src/motion.o \
./src/sensor.o \
./src/stm32f4xx_it.o \
./src/syscalls.o \
./src/system_stm32f4xx.o \
./src/tm_stm32f4_exti.o \
./src/tm_stm32f4_gpio.o \
./src/usart.o \
./src/utils.o 

C_DEPS += \
./src/delay.d \
./src/main.d \
./src/motion.d \
./src/sensor.d \
./src/stm32f4xx_it.d \
./src/syscalls.d \
./src/system_stm32f4xx.d \
./src/tm_stm32f4_exti.d \
./src/tm_stm32f4_gpio.d \
./src/usart.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DOS_USE_SEMIHOSTING -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F407xx -DUSE_HAL_DRIVER -DHSE_VALUE=8000000 -I"/home/radipta/workspace/OMNi/system/include/core" -I"/home/radipta/workspace/OMNi/system/include/peripherals" -I"/home/radipta/workspace/OMNi/include" -I"../include" -I"../system/include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


