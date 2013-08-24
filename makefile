TOOL = arm-none-eabi
CFLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding
LINKER_FLAGS = --no-wchar-size-warning --no-undefined

# Make sure gcc searches the include folder
C_INCLUDE_PATH=include
export C_INCLUDE_PATH

LIBRARIES = csud
BUILD_DIR = bin
SOURCE_DIR = source

CSOURCE := $(wildcard $(SOURCE_DIR)/*.c)
ASOURCE := $(wildcard $(SOURCE_DIR)/*.s)

_OBJECT := $(patsubst %.c,%.o, $(CSOURCE))
_OBJECT += $(patsubst %.s,%.o, $(ASOURCE))
OBJECT = $(addprefix $(BUILD_DIR)/, $(notdir $(_OBJECT)))

all: kernel

# Create the final binary
kernel: theelf
	$(TOOL)-objcopy $(BUILD_DIR)/kernel.elf -O binary $(BUILD_DIR)/kernel.img

# Link all of the objects
theelf: $(OBJECT)
	$(TOOL)-ld $(LINKER_FLAGS) $(OBJECT) -L. -l $(LIBRARIES) -Map $(BUILD_DIR)/kernel.map -T memorymap -o $(BUILD_DIR)/kernel.elf

#build c files
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(TOOL)-gcc -c $< -o $@ $(CFLAGS) 

#build s files (Assembly)
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.s
	$(TOOL)-as $< -o $@
	