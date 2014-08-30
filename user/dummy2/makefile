#######################################################
#         PiOS Application Makefile template          #
#   Update these variables on a per application basis #
#######################################################
APP_NAME=dummy2
#######################################################
# End per application variables                       #
#######################################################
TOOL = arm-none-eabi

# Directories
C_INCLUDE_PATH=include
export C_INCLUDE_PATH

LIBRARIES = 
BUILD_DIR = bin
SOURCE_DIR = source
OBJ_DIR = $(BUILD_DIR)/obj

# Flags
# Note that this creates a relocatable ELF (Required for PiOS)
CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding --no-common -Wpadded -march=armv6j -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -fno-omit-frame-pointer -fpic
LINKER_FLAGS = --no-wchar-size-warning --no-undefined --Map $(BUILD_DIR)/$(APP_NAME).map -T memorymap -pie --nmagic

CHEADERS := $(wildcard include/*.h)
CSOURCE := $(wildcard $(SOURCE_DIR)/*.c)
_COBJECT := $(patsubst %.c,%.o, $(CSOURCE))
COBJECT = $(addprefix $(OBJ_DIR)/, $(notdir $(_COBJECT)))

.PHONY: directories

App: directories $(BUILD_DIR)/$(APP_NAME).img

# Create the final binary
$(BUILD_DIR)/$(APP_NAME).img: $(BUILD_DIR)/$(APP_NAME).elf $(BUILD_DIR)/symbols.txt $(BUILD_DIR)/disassembly.txt
	@$(TOOL)-objcopy $(BUILD_DIR)/$(APP_NAME).elf -O binary $(BUILD_DIR)/$(APP_NAME).img
	
# Create disassembly for ease of debugging
$(BUILD_DIR)/disassembly.txt: $(BUILD_DIR)/$(APP_NAME).elf
	@$(TOOL)-objdump -D $< > $@
	
# Dump symbol table for functions
$(BUILD_DIR)/symbols.txt: $(BUILD_DIR)/$(APP_NAME).elf
	@$(TOOL)-objdump -t $< | awk -F ' ' '{if(NF >= 2) print $$(1), "\t", $$(NF);}' > $@

# Link all of the objects
$(BUILD_DIR)/$(APP_NAME).elf: $(COBJECT)
	@$(TOOL)-ld $(LINKER_FLAGS) $(COBJECT) -o $(BUILD_DIR)/$(APP_NAME).elf
	
# Build C files
$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.c $(CHEADERS)
	@$(TOOL)-gcc -c $< -o $@ $(CFLAGS) 

# Create directories
directories:
	@mkdir -p $(OBJ_DIR)
	
.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)/*