################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ini/config.c \
../ini/dictionary.c \
../ini/ini_main.c \
../ini/iniparser.c \
../ini/load.c 

OBJS += \
./ini/config.o \
./ini/dictionary.o \
./ini/ini_main.o \
./ini/iniparser.o \
./ini/load.o 

C_DEPS += \
./ini/config.d \
./ini/dictionary.d \
./ini/ini_main.d \
./ini/iniparser.d \
./ini/load.d 


# Each subdirectory must supply rules for building sources it contributes
ini/%.o: ../ini/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


