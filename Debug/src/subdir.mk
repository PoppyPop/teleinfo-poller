################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/global.cpp \
../src/teleinfo-message.cpp \
../src/teleinfo-poller.cpp 

OBJS += \
./src/global.o \
./src/teleinfo-message.o \
./src/teleinfo-poller.o 

CPP_DEPS += \
./src/global.d \
./src/teleinfo-message.d \
./src/teleinfo-poller.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


