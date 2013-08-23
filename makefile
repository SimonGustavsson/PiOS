TOOL = arm-none-eabi
CFLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding

C_INCLUDE_PATH=include
export C_INCLUDE_PATH

all: bin/kernel.img

bin/kernel.img: bin/kernel.elf
	$(TOOL)-objcopy bin/kernel.elf -O binary bin/kernel.img

# NOTE: I added --no-wchar-size-warning because it was giving me warnings in the driver.
# Not sure how important this is right now, TODO: Investigate
bin/kernel.elf: bin/start.o bin/gpio.o bin/mailbox.o bin/framebuffer.o bin/stdio.o bin/stringutil.o bin/terminal.o bin/timer.o bin/main.o
	$(TOOL)-ld --no-wchar-size-warning --no-undefined bin/start.o bin/gpio.o bin/timer.o bin/mailbox.o bin/framebuffer.o bin/stdio.o bin/stringutil.o bin/terminal.o bin/main.o -L. -l csud -Map bin/mapfile.map -T memorymap -o bin/kernel.elf

# The C Files (Not happy about this but dependencies makes my brain hurt and I'm tired)
bin/gpio.o: source/gpio.c
	$(TOOL)-gcc -c -o $@ $< $(CFLAGS)
bin/mailbox.o: source/mailbox.c
	$(TOOL)-gcc -c -o $@ $< $(CFLAGS)
bin/framebuffer.o: source/framebuffer.c
	$(TOOL)-gcc -c -o $@ $< $(CFLAGS)
bin/terminal.o: source/terminal.c
	$(TOOL)-gcc -c -o $@ $< $(CFLAGS)
bin/stringutil.o: source/stringutil.c
	$(TOOL)-gcc -c -o $@ $< $(CFLAGS)
bin/stdio.o: source/stdio.c
	$(TOOL)-gcc -c -o $@ $< $(CFLAGS)
bin/timer.o: source/timer.c
	$(TOOL)-gcc -c -o $@ $< $(CFLAGS)	
bin/main.o: source/main.c
	$(TOOL)-gcc -c -o $@ $< $(CFLAGS)
	
# The assembly files
bin/start.o: source/start.s
	arm-none-eabi-as source/start.s -o bin/start.o
