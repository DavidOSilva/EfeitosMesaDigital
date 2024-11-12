################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccs1270/ccs/tools/compiler/c5500_4.4.1/bin/cl55" -v5502 --memory_model=large -g --include_path="C:/Users/caios/workspace_v12/teste6" --include_path="C:/ti/ccs1270/ccs/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/caios/Documents/laboratorio integrado III/ezdsp5502_v1/include" --include_path="C:/Users/caios/Documents/laboratorio integrado III/ezdsp5502_v1/C55xxCSL/include" --include_path="C:/ti/ccs1270/ccs/tools/compiler/c5500_4.4.1/include" --define=c5502 --display_error_number --diag_warning=225 --ptrdiff_size=32 --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.s55 $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccs1270/ccs/tools/compiler/c5500_4.4.1/bin/cl55" -v5502 --memory_model=large -g --include_path="C:/Users/caios/workspace_v12/teste6" --include_path="C:/ti/ccs1270/ccs/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/caios/Documents/laboratorio integrado III/ezdsp5502_v1/include" --include_path="C:/Users/caios/Documents/laboratorio integrado III/ezdsp5502_v1/C55xxCSL/include" --include_path="C:/ti/ccs1270/ccs/tools/compiler/c5500_4.4.1/include" --define=c5502 --display_error_number --diag_warning=225 --ptrdiff_size=32 --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


