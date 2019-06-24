################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include timer/subdir.mk
-include log/subdir.mk
-include ini/subdir.mk
-include epoll/subdir.mk
-include design/subdir.mk
-include datastruct/subdir.mk
-include cstring/subdir.mk
-include cert/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: udpserver

# Tool invocations
udpserver: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "udpserver" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) udpserver
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
