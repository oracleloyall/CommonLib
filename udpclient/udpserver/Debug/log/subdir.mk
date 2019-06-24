################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../log/condition.c \
../log/plog.c 

OBJS += \
./log/condition.o \
./log/plog.o 

C_DEPS += \
./log/condition.d \
./log/plog.d 


# Each subdirectory must supply rules for building sources it contributes
log/%.o: ../log/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


