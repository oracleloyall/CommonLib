################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../linux/net/async_task_queue.c \
../linux/net/buffer.c \
../linux/net/channel.c \
../linux/net/inetaddr.c \
../linux/net/log.c \
../linux/net/loop.c \
../linux/net/socket.c \
../linux/net/time_wheel.c \
../linux/net/timer_queue.c \
../linux/net/udp_peer.c \
../linux/net/util.c 

O_SRCS += \
../linux/net/async_task_queue.o \
../linux/net/buffer.o \
../linux/net/channel.o \
../linux/net/inetaddr.o \
../linux/net/loop.o \
../linux/net/socket.o \
../linux/net/tcp_client.o \
../linux/net/tcp_connection.o \
../linux/net/tcp_server.o \
../linux/net/timer_queue.o \
../linux/net/udp_peer.o 

OBJS += \
./linux/net/async_task_queue.o \
./linux/net/buffer.o \
./linux/net/channel.o \
./linux/net/inetaddr.o \
./linux/net/log.o \
./linux/net/loop.o \
./linux/net/socket.o \
./linux/net/time_wheel.o \
./linux/net/timer_queue.o \
./linux/net/udp_peer.o \
./linux/net/util.o 

C_DEPS += \
./linux/net/async_task_queue.d \
./linux/net/buffer.d \
./linux/net/channel.d \
./linux/net/inetaddr.d \
./linux/net/log.d \
./linux/net/loop.d \
./linux/net/socket.d \
./linux/net/time_wheel.d \
./linux/net/timer_queue.d \
./linux/net/udp_peer.d \
./linux/net/util.d 


# Each subdirectory must supply rules for building sources it contributes
linux/net/%.o: ../linux/net/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


