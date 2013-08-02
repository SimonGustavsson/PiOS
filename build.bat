arm-none-eabi-as source/start.s -o build/start.o

arm-none-eabi-gcc -Wall -O2 -nostdlib -nostartfiles -ffreestanding -c source/gpio.c -o build/gpio.o
arm-none-eabi-gcc -Wall -O2 -nostdlib -nostartfiles -ffreestanding -c source/mailbox.c -o build/mailbox.o
arm-none-eabi-gcc -Wall -O2 -nostdlib -nostartfiles -ffreestanding -c source/framebuffer.c -o build/framebuffer.o
arm-none-eabi-gcc -Wall -O2 -nostdlib -nostartfiles -ffreestanding -c source/timer.c -o build/timer.o
arm-none-eabi-gcc -Wall -O2 -nostdlib -nostartfiles -ffreestanding -c source/main.c -o build/main.o

arm-none-eabi-ld build/start.o build/gpio.o build/timer.o build/mailbox.o build/framebuffer.o build/main.o -T memorymap -o build/kernel.elf

arm-none-eabi-objcopy build/kernel.elf -O binary build/kernel.img
