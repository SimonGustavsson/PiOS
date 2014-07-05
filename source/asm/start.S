#include "memory_map.h"

.type reset %function
.section .text.boot
.globl reset
reset:
    ;@ Args from bootloader
    mov r4, r0 ;@ Machine type (Rpi bootloader)
    mov r5, r1 ;@ Atags PA (Rpi bootloader)
    mov r6, r2 ;@ Debugging symbols (PiOS Loader)

	mov r0,#0x8000
    mov r1,#0x0000
	
	;@ Setup the stacks for the different interrupt modes (make sure interrupts are disabled first)
	;@ IRQ
    mov r0,#0xD2 ;@ PSR_IRQ_MODE (0x12) | PSR_FIQ_DIS(0x40) | PSR_IRQ_DIS (0x80)
    msr cpsr_c,r0
    ldr sp, =IRQ_STACK_PA_START

	;@ FIQ
    mov r0,#0xD1 ;@ PSR_FIQ_MODE (0x11) |PSR_FIQ_DIS (0x40) | PSR_IRQ_DIS (0x80)
    msr cpsr_c,r0
    ldr sp, =FIQ_STACK_PA_START

	;@ ABORT PSR_ABORT_MODE (0x17) | PSR_FIQ_DIS (0x40) | PSR_IRQ_DIS (0x80)
	mov r0, #0xD7
	msr cpsr_c,r0
	ldr sp, =ABORT_STACK_PA_START

	;@ SYSTEM PSR_SYSTEM_MODE (0x1F) | PSR_FIQ_DIS(0x40) | PSR_IRQ_DIS (0x80)
	mov r0, #0xDF ;@ 001 1111
	msr cpsr_c, r0
	ldr sp, =SM_STACK_PA_START

	;@ UNDEFINED PSR_UNDEFINED_MODE (0x1B) | PSR_FIQ_DIS (0x40) | PSR_IRQ_DIS (0x80)
	mov r0, #0xDB
	msr cpsr_c, r0
	ldr sp, =UD_STACK_PA_START
	        
	;@ SVC
    mov r0,#0xD3 ;@ PSR_SVC_MODE (0x13) PSR_FIQ_DIS (0x40) | PSR_IRQ_DIS (0x80)
    msr cpsr_c,r0
    ldr sp, =SVC_STACK_PA_START
	
    ;@ Pass in the args
    mov r0, r4
    mov r1, r5
    mov r2, r6

	;@ Kick of system initialization (init.c)
	ldr r3, =sysinit_stage1
	blx r3

;@ cmain should never return, just hang around (heh.. get it?)

.type hang %function
.section .text.boot
.globl hang
hang: 
	b hang
