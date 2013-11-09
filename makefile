TOOL = arm-none-eabi
CFLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding --no-common -Wpadded
LINKER_FLAGS = --no-wchar-size-warning --no-undefined

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
$(BUILD_DIR)/kernel.img: bin/kernel.elf
	@$(TOOL)-objcopy $(BUILD_DIR)/kernel.elf -O binary $(BUILD_DIR)/kernel.img

# Link all of the objects
$(BUILD_DIR)/kernel.elf: $(AOBJECT) $(COBJECT) libcsud.a
	@$(TOOL)-ld $(LINKER_FLAGS) $(AOBJECT) $(COBJECT) -L. -l $(LIBRARIES) -Map $(BUILD_DIR)/kernel.map -T memorymap -o $(BUILD_DIR)/kernel.elf

#build c files
$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.c $(CHEADERS)
	@$(TOOL)-gcc -c $< -o $@ $(CFLAGS) 

#build s files (Assembly)
$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.s
	@$(TOOL)-as $< -o $@
	
directories:
	@mkdir -p $(OBJ_DIR)
	
.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)/*