################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cert/crypt.c 

OBJS += \
./cert/crypt.o 

C_DEPS += \
./cert/crypt.d 


# Each subdirectory must supply rules for building sources it contributes
cert/%.o: ../cert/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<" -I../
	@echo 'Finished building: $<'
	@echo ' '


