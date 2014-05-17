#include "memory_map.h"

.section .text.boot
.globl reset
reset:
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
	
	;@ Kick of system initialization (init.c)
	ldr r0, =sysinit_stage1
	blx r0

;@ cmain should never return, just hang around (heh.. get it?)
.section .text.boot
.globl hang
hang: 
	b hang
