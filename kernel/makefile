TOOL = arm-none-eabi
CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding --no-common -march=armv6j -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -fno-omit-frame-pointer -mno-thumb-interwork -mlong-calls -marm -g -std=gnu11
LINKER_FLAGS = --no-wchar-size-warning --no-undefined

ASSEMBLER_FLAGS = -march=armv6j  -mfpu=vfp -mfloat-abi=hard

ifdef TARGET
	TARGET_ARCH = TARGET
else
	TARGET_ARCH = $(shell ../default-target.sh)
endif

# Currently only support bcm2835
ifneq ($(TARGET_ARCH), bcm2835)
    $(error Unsupported target architecture, see 'make help' for a list of supported architectures)
endif

LIBRARIES =
BUILD_DIR = bin
SOURCE_DIR = source
INCLUDE_DIR = include
OBJ_DIR := $(BUILD_DIR)/obj
ARCH_DIR := arch/$(TARGET_ARCH)
DEPENDENCY_DIR := $(BUILD_DIR)/dependencies
LINK_SCRIPT_SRC = arch/$(TARGET_ARCH)/linker.c
LINK_SCRIPT = $(BUILD_DIR)/memory.ld

# Make sure gcc searches the include folder
C_INCLUDE_PATH := $(shell find $(INCLUDE_DIR)/ -type d)
C_INCLUDE_PATH += $(shell find $(SOURCE_DIR)/ -type d)
C_INCLUDE_PATH += $(shell find $(ARCH_DIR)/include -type d)

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
CSOURCE += $(shell find $(ARCH_DIR)/source/ -name '*.c')

ASOURCE := $(shell find $(SOURCE_DIR)/ -name '*.s')
ASOURCE += $(shell find $(SOURCE_DIR)/ -name '*.S')
ASOURCE += $(shell find $(ARCH_DIR)/source/ -name '*.s')
ASOURCE += $(shell find $(ARCH_DIR)/source/ -name '*.S')

_COBJECT := $(patsubst %.c,%.o, $(CSOURCE))
_AOBJECT := $(patsubst %.S,%.o, $(patsubst %.s,%.o, $(ASOURCE)))
AOBJECT := $(addprefix $(OBJ_DIR)/, $(notdir $(_AOBJECT)))
COBJECT := $(addprefix $(OBJ_DIR)/, $(_COBJECT))

# To figure out why objects are being build, uncomment this:
#OLD_SHELL := $(SHELL)
#SHELL = $(warning [Evaluating target:'$@' Prereqs:'$^' Never:'$?'])$(OLD_SHELL)

.PHONY: directories

PiOS: directories $(BUILD_DIR)/kernel.img

# Create the final binary
$(BUILD_DIR)/kernel.img: $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.elf.debug
	@echo Creating flat binary...
	@$(TOOL)-objcopy $(BUILD_DIR)/kernel.elf -O binary $(BUILD_DIR)/kernel.img
	@echo Kernel built for $(TARGET_ARCH)

# Create disassembly for ease of debugging
debug:
	@echo Dumping disassembly...
	@$(TOOL)-objdump -D $(BUILD_DIR)/kernel.elf > $(BUILD_DIR)/disassembly

	@echo Dumping symbol table...
	@$(TOOL)-objdump -t $(BUILD_DIR)/kernel.elf | awk -F ' ' '{if(NF >= 2) print $$(1), "\t", $$(NF);}' > $(BUILD_DIR)/symbols.txt

# Extract debugging symbols into separate file
$(BUILD_DIR)/kernel.elf.debug: $(BUILD_DIR)/kernel.elf
	@$(TOOL)-objcopy --only-keep-debug $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.elf.debug
#	@$(TOOL)-objcopy --strip-debug $(BUILD_DIR)/kernel.elf

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
$(OBJ_DIR)/%.o: %.c
	@echo Building $<
	
# Create the output directories if they don't exist
	@mkdir $(DEPENDENCY_DIR)/$(dir $*) -p
	@mkdir $(OBJ_DIR)/$(dir $*) -p

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
	@$(TOOL)-gcc -P -E -c $< -o $@.s $(ASSEMBLER_FLAGS) $(GCC_INCLUDE)
	@$(TOOL)-as -c $@.s -o $@ $(ASSEMBLER_FLAGS)
	@rm $@.s

directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(DEPENDENCY_DIR)

.PHONY: clean debug help fire

clean:
	@rm -rfv $(BUILD_DIR)/*
	@rm -rfv $(BUILD_DIR)

fire:
	@echo "Heh..."

help:
	@echo "MakeFile for PiOS"
	@echo
	@echo "Targets:"
	@echo "	pios  - Builds the kernel"
	@echo "	debug - Generates disassembly and Symbol Table"
	@echo "	clean - Removes all built files"
	@echo "Parameters:"
	@echo "	TARGET:"
	@echo "		bcm2835 (Raspberry Pi)"