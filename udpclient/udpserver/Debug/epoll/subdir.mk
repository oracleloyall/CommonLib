################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../epoll/async_task_queue.c \
../epoll/buffer.c \
../epoll/channel.c \
../epoll/inetaddr.c \
../epoll/log.c \
../epoll/loop.c \
../epoll/socket.c \
../epoll/time_wheel.c \
../epoll/timer_queue.c \
../epoll/udp_peer.c \
../epoll/util.c 

OBJS += \
./epoll/async_task_queue.o \
./epoll/buffer.o \
./epoll/channel.o \
./epoll/inetaddr.o \
./epoll/log.o \
./epoll/loop.o \
./epoll/socket.o \
./epoll/time_wheel.o \
./epoll/timer_queue.o \
./epoll/udp_peer.o \
./epoll/util.o 

C_DEPS += \
./epoll/async_task_queue.d \
./epoll/buffer.d \
./epoll/channel.d \
./epoll/inetaddr.d \
./epoll/log.d \
./epoll/loop.d \
./epoll/socket.d \
./epoll/time_wheel.d \
./epoll/timer_queue.d \
./epoll/udp_peer.d \
./epoll/util.d 


# Each subdirectory must supply rules for building sources it contributes
epoll/%.o: ../epoll/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


