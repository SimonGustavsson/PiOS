TOOL = arm-none-eabi
CFLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding --no-common -Wpadded -march=armv6j -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -fno-omit-frame-pointer
LINKER_FLAGS = --no-wchar-size-warning --no-undefined

ASSEMBLER_FLAGS = -march=armv6j  -mfpu=vfp -mfloat-abi=hard

# Make sure gcc searches the include folder
C_INCLUDE_PATH=include;csud\include
export C_INCLUDE_PATH

LIBRARIES = csud
BUILD_DIR = bin
SOURCE_DIR = source
OBJ_DIR = $(BUILD_DIR)/obj

# TODO, include all include directories in CHEADERS
CHEADERS := $(wildcard include/*.h)
CSOURCE := $(wildcard $(SOURCE_DIR)/*.c)
ASOURCE := $(wildcard $(SOURCE_DIR)/*.s)

_COBJECT := $(patsubst %.c,%.o, $(CSOURCE))
_AOBJECT := $(patsubst %.s,%.o, $(ASOURCE))
AOBJECT = $(addprefix $(OBJ_DIR)/, $(notdir $(_AOBJECT)))
COBJECT = $(addprefix $(OBJ_DIR)/, $(notdir $(_COBJECT)))

.PHONY: directories

PiOS: directories $(BUILD_DIR)/kernel.img

# Create the final binary
$(BUILD_DIR)/kernel.img: $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/symbols.txt
	@$(TOOL)-objcopy $(BUILD_DIR)/kernel.elf -O binary $(BUILD_DIR)/kernel.img
	
# Dump symbol table for functions
$(BUILD_DIR)/symbols.txt: $(BUILD_DIR)/kernel.elf
	@$(TOOL)-objdump -t $< | awk -F ' ' '{if(NF >= 2) print $$(1), "\t", $$(NF);}' > $@

# Link all of the objects (Temporarily removed -l $(LIBRARIES))
$(BUILD_DIR)/kernel.elf: $(AOBJECT) $(COBJECT) libcsud.a
	@$(TOOL)-ld $(LINKER_FLAGS) $(AOBJECT) $(COBJECT) -Map $(BUILD_DIR)/kernel.map -T memorymap -o $(BUILD_DIR)/kernel.elf

#build c files
$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.c $(CHEADERS)
	@$(TOOL)-gcc -c $< -o $@ $(CFLAGS) 

#build s files (Assembly)
$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.s
	@$(TOOL)-as $(ASSEMBLER_FLAGS) $< -o $@
	
directories:
	@mkdir -p $(OBJ_DIR)
	
.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)/*