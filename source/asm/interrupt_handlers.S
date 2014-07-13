;@
;@ Interrupt vector functions - do NOT call these from C code!
;@ 

.type irq %function
.globl irq
irq:
    sub lr, #4         ;@ (Required to return from exception)

    ;@ Save context
    push {r0-r12, lr}  ;@ Save registers onto IRQ stack
    
    mrs r0, spsr
    ;@msr cpsr_c, #0xD3 ;@ Hop into SVC mode to save banked registers
    cps #0x13           ;@ SVC mode
    mov r1, sp
    mov r2, lr

    ;@msr cpsr_c, #0xD2;@ Get back to IRQ mode
    cps #0x12          ;@ IRC mode
    push {r0-r2}       ;@ Push values to IRQ stack

    mov r0, sp         ;@ Pass in a pointer to the registers as a param to the irq handler
    add r0, #4

    bl c_irq_handler   ;@ Osdev note: All this does is print out the registers struct

    ;@ Restore context
    pop {r0-r2}        ;@ 
    msr spsr, r0       ;@ Restore banked registers

    msr cpsr_c, #0xD3  ;@ Get into SVC mode
    cps #0x13          ;@ SVC mode
    mov sp, r1
    mov lr, r2

    ;@msr cpsr_c, #0xD2  ;@ Hop back into IRQ mode
    cps #0x12

    pop  {r0-r12,lr}
	
    movs pc, lr

.type data_abort %function
.globl data_abort
data_abort:
    ;@ Proloque
    push {fp, lr}
    add fp, sp, #4
    sub sp, sp, #8

	push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    
	;@ Pass the address where it happened as the 1st argument
	subs r0, lr, #8

	;@ Pass the error type as the 2nd argument
	mrc p15, 0, r1, c5, c0, 0
	and r1, r1, #0xF
	
	;@ Pass the address that was accessed as the 3rd argumnet
	mrc p15, 0, r2, c6, c0, 0

    ;@ Pass the Fault Status Register as the 4th argument
    mrc p15, 0, r3, c5, c0, 0

	bl c_abort_data_handler
	
	pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
	
    sub sp, fp, #4
    pop {fp, lr}

	subs PC, lr, #4

    
.type instruction_abort %function
.globl instruction_abort
instruction_abort:
	push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    
	;@ Get the address that caused it
	subs r0, lr, #4

	;@ Get the error type
	mrc p15, 0, r1, c5, c0, 0
	and r1, r1, #0xF

	bl c_abort_instruction_handler
	
	pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
	
	subs PC, lr, #4

    
.type undefined %function
.globl undefined
undefined:
    push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
	
    mov r0, lr

	bl c_undefined_handler
	
    pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}

	subs PC, lr, #4

    
.type swi %function
.globl swi
swi:
    ;@ Save registers and LR onto stack
	stmfd sp!, {r4-r5,lr}

    ;@ Don't touch r0-r2 as they contain arguments
    ;@ To the SWI

	;@ SWI number is stored in top 8 bits of the instruction
	ldr r3, [lr, #-4]
	bic r3, r3, #0xFF000000

	bl c_swi_handler

	;@ Restore registers and return
	LDMFD sp!, {r4, r5, pc}^
    