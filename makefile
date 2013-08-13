TOOL = arm-none-eabi
CFLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding

all: bin/kernel.img

bin/kernel.img: bin/kernel.elf
	$(TOOL)-objcopy bin/kernel.elf -O binary bin/kernel.img

bin/kernel.elf: bin/start.o bin/gpio.o bin/mailbox.o bin/framebuffer.o bin/timer.o bin/main.o
	$(TOOL)-ld bin/start.o bin/gpio.o bin/timer.o bin/mailbox.o bin/framebuffer.o bin/main.o -T memorymap -o bin/kernel.elf

# The C Files (Not happy about this but dependencies makes my brain hurt and I'm tired)
bin/gpio.o: source/gpio.c
	$(TOOL)-gcc -c -o $@ $< $(CFLAGS)
bin/mailbox.o: source/mailbox.c
	$(TOOL)-gcc -c -o $@ $< $(CFLAGS)
bin/framebuffer.o: source/framebuffer.c
	$(TOOL)-gcc -c -o $@ $< $(CFLAGS)
bin/timer.o: source/timer.c
	$(TOOL)-gcc -c -o $@ $< $(CFLAGS)	
bin/main.o: source/main.c
	$(TOOL)-gcc -c -o $@ $< $(CFLAGS)
	
# The assembly files
bin/start.o: source/start.s
	arm-none-eabi-as source/start.s -o bin/start.o
