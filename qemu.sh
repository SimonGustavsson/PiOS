#!/bin/bash
qemu-system-arm -kernel bin/kernel.elf -cpu arm1176 -m 512 -M raspi -no-reboot -serial stdio -s

