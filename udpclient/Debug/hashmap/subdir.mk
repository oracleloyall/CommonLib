################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../hashmap/hashmap.c 

OBJS += \
./hashmap/hashmap.o 

C_DEPS += \
./hashmap/hashmap.d 


# Each subdirectory must supply rules for building sources it contributes
hashmap/%.o: ../hashmap/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


