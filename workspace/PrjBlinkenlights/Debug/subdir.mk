################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../color.c \
../infomem.c \
../lcd.c \
../main.c \
../menu.c \
../utils.c 

OBJS += \
./color.o \
./infomem.o \
./lcd.o \
./main.o \
./menu.o \
./utils.o 

C_DEPS += \
./color.d \
./infomem.d \
./lcd.d \
./main.d \
./menu.d \
./utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	msp430-gcc -O0 -g3 -Wall -c -fmessage-length=0 -mmcu=msp430g2231 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


