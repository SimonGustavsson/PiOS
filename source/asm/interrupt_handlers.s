;@
;@ Interrupt vector functions - do NOT call these from C code!
;@ 

.globl irq
irq:
    ;@ We have no idea what might be in these registers, so make sure they're
	;@ saved so we can go back to the previous state once the interrupt has been handled
	;@ TODO: Use STMFD?
    push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}

    ;@ Pass in a pointer to the registers as a param to the irq handler
    mov r0, sp

	;@ Jump to C Handler
    bl c_irq_handler

	;@ TODO: Use LDMFD?
    pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
	
	;@ Due to the CPU pipeline, we have to manipulate the return address
	;@ The value stored in LR will include an offset which we need to subtract
	;@ Offset: FIQ=4, IRQ=4, Pre-Fetch=4, SWI=0, Undefined=0, DataAbort=8, Reset=n/a
    subs pc,lr,#4

.globl data_abort
data_abort:	
	push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    
	;@ Address where it happened
	subs r0, lr, #8

	;@ Get the error type
	mrc p15, 0, r1, c5, c0, 0
	and r1, r1, #0xF
	
	;@ Get address that was accessed
	mrc p15, 0, r2, c6, c0, 0

    ;@ Throw in the stack pointer aswell for stack trace
    mov r3, sp

	bl c_abort_data_handler
	
	pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
	
	subs PC, lr, #4

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

.globl undefined
undefined:
    push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
	
    mov r0, lr

	bl c_undefined_handler
	
    pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}

	subs PC, lr, #4

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
    