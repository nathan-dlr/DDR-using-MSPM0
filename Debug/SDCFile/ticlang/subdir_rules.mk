################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
SDCFile/ticlang/%.o: ../SDCFile/ticlang/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/ti/ccs1240/ccs/tools/compiler/ti-cgt-armllvm_2.1.3.LTS/bin/tiarmclang" -c -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"/Users/nathan/Desktop/MSPM0_ValvanoWare/ECE319K_Lab9H" -I"/Users/nathan/Desktop/MSPM0_ValvanoWare/ECE319K_Lab9H/Debug" -I"/Applications/ti/mspm0_sdk_1_20_01_06/source/third_party/CMSIS/Core/Include" -I"/Applications/ti/mspm0_sdk_1_20_01_06/source" -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"SDCFile/ticlang/$(basename $(<F)).d_raw" -MT"$(@)" -I"/Users/nathan/Desktop/MSPM0_ValvanoWare/ECE319K_Lab9H/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


