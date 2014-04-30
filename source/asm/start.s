#include "memory_map.h"

.section .text.boot
.globl _start
_start:
	;@ Setup interrupt vector	
	ldr pc,reset_handler
	ldr pc,undefined_handler
	ldr pc,swi_handler
	ldr pc,prefetch_handler
	ldr pc,data_handler
	ldr pc,unused_handler
	ldr pc,irq_handler
	ldr pc,fiq_handler
    
	;@ Route handlers to the appropriate function
	reset_handler:      .word reset
	undefined_handler:  .word undefined
	swi_handler:        .word swi
	prefetch_handler:   .word instruction_abort
	data_handler:       .word data_abort
	unused_handler:     .word hang
	irq_handler:        .word irq
	fiq_handler:        .word hang
	
reset:
	mov r0,#0x8000
    mov r1,#0x0000
	
	;@ The interrupt branch instructions are placed at 0x8000
	;@ Move them to 0x0000 where the CPU expects the interrupt vector to be
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
	
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
	
	;@ Jump into main function in C (main.c)
	bl cmain

;@ cmain should never return, just hang around (heh.. get it?)
hang: 
	b hang
