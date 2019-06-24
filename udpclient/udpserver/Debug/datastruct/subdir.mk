################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../datastruct/hashmapi.c \
../datastruct/vector.c 

OBJS += \
./datastruct/hashmapi.o \
./datastruct/vector.o 

C_DEPS += \
./datastruct/hashmapi.d \
./datastruct/vector.d 


# Each subdirectory must supply rules for building sources it contributes
datastruct/%.o: ../datastruct/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


