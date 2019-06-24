################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Callback.c \
../UDP_server.c \
../balloc.c \
../epoll_server.c \
../function.c \
../hashmap.c \
../jsw_rbtree.c \
../main.c \
../thpool.c

OBJS += \
./Callback.o \
./UDP_server.o \
./balloc.o \
./epoll_server.o \
./function.o \
./hashmap.o \
./jsw_rbtree.o \
./main.o \
./thpool.o 

C_DEPS += \
./Callback.d \
./UDP_server.d \
./balloc.d \
./epoll_server.d \
./function.d \
./hashmap.d \
./jsw_rbtree.d \
./main.d \
./thpool.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -I../ -L../libcrypto.a -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


