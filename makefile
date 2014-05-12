TOOL = arm-none-eabi
CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding --no-common -Wpadded -march=armv6j -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -fno-omit-frame-pointer -mno-thumb-interwork -mlong-calls -marm -g
LINKER_FLAGS = --no-wchar-size-warning --no-undefined

ASSEMBLER_FLAGS = -march=armv6j  -mfpu=vfp -mfloat-abi=hard

LIBRARIES = csud
BUILD_DIR = bin
SOURCE_DIR = source
INCLUDE_DIR = include
OBJ_DIR := $(BUILD_DIR)/obj
DEPENDENCY_DIR := $(BUILD_DIR)/dependencies
LINK_SCRIPT_SRC = memorymap.c
LINK_SCRIPT = $(BUILD_DIR)/memory.ld

# Make sure gcc searches the include folder

#C_INCLUDE_PATH := include csud/include
C_INCLUDE_PATH := $(shell find $(INCLUDE_DIR)/ -type d)
C_INCLUDE_PATH += $(shell find $(SOURCE_DIR)/ -type d)

GCC_INCLUDE = $(foreach d, $(C_INCLUDE_PATH), -I$d)

export C_INCLUDE_PATH

# When building C files, look in all subdirectories of source
#VPATH := $(shell find $(SOURCE_DIR)/ -type d)
# (And headers too!)
#VPATH += $(shell find $(INCLUDE_DIR)/ -type d)
null      :=
SPACE     := $(null) $(null)
VPATH := $(subst $(SPACE),:,$(C_INCLUDE_PATH))

CHEADERS := $(shell find $(INCLUDE_DIR)/ -name '*.h')
CSOURCE := $(shell find $(SOURCE_DIR)/ -name '*.c')
ASOURCE := $(shell find $(SOURCE_DIR)/ -name '*.s')
ASOURCE += $(shell find $(SOURCE_DIR)/ -name '*.S')

_COBJECT := $(patsubst %.c,%.o, $(CSOURCE))
_AOBJECT := $(patsubst %.S,%.o, $(patsubst %.s,%.o, $(ASOURCE)))
AOBJECT := $(addprefix $(OBJ_DIR)/, $(notdir $(_AOBJECT)))
COBJECT := $(addprefix $(OBJ_DIR)/, $(notdir $(_COBJECT)))

# To figure out why objects are being build, uncomment this:
#OLD_SHELL := $(SHELL)
#SHELL = $(warning [Evaluating target:'$@' Prereqs:'$^' Never:'$?'])$(OLD_SHELL)

.PHONY: directories

PiOS: directories $(BUILD_DIR)/kernel.img

# Create the final binary
$(BUILD_DIR)/kernel.img: $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/symbols.txt $(BUILD_DIR)/disassembly.txt
	@echo Creating flat binary...
	@$(TOOL)-objcopy $(BUILD_DIR)/kernel.elf -O binary $(BUILD_DIR)/kernel.img

# Create disassembly for ease of debugging
$(BUILD_DIR)/disassembly.txt: $(BUILD_DIR)/kernel.elf
	@echo Dumping disassembly...
	@$(TOOL)-objdump -D $< > $@
	
# Dump symbol table for functions
$(BUILD_DIR)/symbols.txt: $(BUILD_DIR)/kernel.elf
	@echo Dumping symbol table..
	@$(TOOL)-objdump -t $< | awk -F ' ' '{if(NF >= 2) print $$(1), "\t", $$(NF);}' > $@

# Link all of the objects (Temporarily removed -l $(LIBRARIES))
$(BUILD_DIR)/kernel.elf: $(AOBJECT) $(COBJECT) $(LINK_SCRIPT)
	@echo Linking kernel.elf...
	@$(TOOL)-ld $(LINKER_FLAGS) $(AOBJECT) $(COBJECT) $(GCC_INCLUDE) -Map $(BUILD_DIR)/kernel.map -T $(LINK_SCRIPT) -o $(BUILD_DIR)/kernel.elf

# Run the linker script through the preprocessor
$(LINK_SCRIPT): $(LINK_SCRIPT_SRC)
	@echo Creating linker script...
	@$(TOOL)-gcc $< -o $@ -E -P $(GCC_INCLUDE)

# If make was run previously, we will have .d dependency files
# Describing wihich headers the objects depend on, import those targets

-include $(addprefix $(DEPENDENCY_DIR)/, $(notdir $(COBJECT:.o=.d)))
	
#build c files
# The dependency stuff here is a hack, shouldn't have to do this?
$(OBJ_DIR)/$(notdir %).o: %.c
	@echo Building $<
	@$(TOOL)-gcc -c $< -o $@ $(CFLAGS) $(GCC_INCLUDE) -MD -MF $(DEPENDENCY_DIR)/$*.d 
	
# Create a temp file
	@mv -f $(DEPENDENCY_DIR)/$*.d $(DEPENDENCY_DIR)/$*.d.tmp
	
# Open dependency file, append OBJ_DIR to target and create empty targets for all dependencies
# So that MAKE doesn't complain if a dependency is removed/renamed and still builds everything that used to depend on it
	@sed -e 's|.*: |$(OBJ_DIR)/$*.o:|' < $(DEPENDENCY_DIR)/$*.d.tmp > $(DEPENDENCY_DIR)/$*.d
	@sed -e 's/.*: //' -e 's/\\$$//' < $(DEPENDENCY_DIR)/$*.d.tmp | fmt -1 | sed -e 's/*//' -e 's/$$/: /' >> $(DEPENDENCY_DIR)/$*.d
	  
# Remove the temp file
	@rm -f $(DEPENDENCY_DIR)/$*.d.tmp

#build s files (Assembly)
$(OBJ_DIR)/%.o: %.s
	@echo Building $<
	@$(TOOL)-as $(ASSEMBLER_FLAGS) $< -o $@
	
# Uppercase S indicates header includes, run it through GCC instead of as
$(OBJ_DIR)/%.o: %.S
	@echo "Building $< (gcc)"
	@$(TOOL)-gcc -c $< -o $@ $(ASSEMBLER_FLAGS) $(GCC_INCLUDE)
	
directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(DEPENDENCY_DIR)
	
.PHONY: clean
    clean:
	@rm -rf $(BUILD_DIR)/*
