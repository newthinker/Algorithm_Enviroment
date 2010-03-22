################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Common.cpp \
../Dimap.cpp \
../GCProjection.cpp \
../GcpMatch.cpp \
../GeometryRectify.cpp \
../Histogram.cpp \
../Image.cpp \
../ImageDriver.cpp \
../Kernel.cpp \
../LeastSquareMatch.cpp \
../Output.cpp \
../PatternMatch.cpp \
../Progress.cpp \
../ProjectWGS84.cpp \
../Projection.cpp \
../RPCParm.cpp \
../RealVector.cpp \
../Reference.cpp \
../TMProjection.cpp \
../Triangle.cpp \
../dem.cpp \
../nsdtfdem.cpp 

OBJS += \
./Common.o \
./Dimap.o \
./GCProjection.o \
./GcpMatch.o \
./GeometryRectify.o \
./Histogram.o \
./Image.o \
./ImageDriver.o \
./Kernel.o \
./LeastSquareMatch.o \
./Output.o \
./PatternMatch.o \
./Progress.o \
./ProjectWGS84.o \
./Projection.o \
./RPCParm.o \
./RealVector.o \
./Reference.o \
./TMProjection.o \
./Triangle.o \
./dem.o \
./nsdtfdem.o 

CPP_DEPS += \
./Common.d \
./Dimap.d \
./GCProjection.d \
./GcpMatch.d \
./GeometryRectify.d \
./Histogram.d \
./Image.d \
./ImageDriver.d \
./Kernel.d \
./LeastSquareMatch.d \
./Output.d \
./PatternMatch.d \
./Progress.d \
./ProjectWGS84.d \
./Projection.d \
./RPCParm.d \
./RealVector.d \
./Reference.d \
./TMProjection.d \
./Triangle.d \
./dem.d \
./nsdtfdem.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


