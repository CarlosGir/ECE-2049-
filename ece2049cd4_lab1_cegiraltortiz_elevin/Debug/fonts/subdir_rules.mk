################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
fonts/%.obj: ../fonts/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-msp430_21.6.0.LTS/bin/cl430" -vmspx --data_model=restricted -Ooff --use_hw_mpy=F5 --include_path="C:/ti/ccs1120/ccs/ccs_base/msp430/include" --include_path="C:/Users/cgira/OneDrive/Documents/Code Composer Studio/ece2049cd4_lab2_cegiraltortiz_elevin" --include_path="C:/Users/cgira/OneDrive/Documents/Code Composer Studio/ece2049cd4_lab2_cegiraltortiz_elevin/grlib" --include_path="C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-msp430_21.6.0.LTS/include" --advice:power_severity=suppress --define=__MSP430F5529__ -g --gcc --printf_support=minimal --diag_warning=225 --display_error_number --abi=eabi --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="fonts/$(basename $(<F)).d_raw" --obj_directory="fonts" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


