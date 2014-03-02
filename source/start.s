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
    mov sp,#0x7900

	;@ FIQ
    mov r0,#0xD1 ;@ PSR_FIQ_MODE (0x11) |PSR_FIQ_DIS (0x40) | PSR_IRQ_DIS (0x80)
    msr cpsr_c,r0
    mov sp,#0x4000

	;@ ABORT PSR_ABORT_MODE (0x17) | PSR_FIQ_DIS (0x40) | PSR_IRQ_DIS (0x80)
	mov r0, #0xD7
	msr cpsr_c,r0
	ldr sp, =0x1208000

	;@ SYSTEM PSR_SYSTEM_MODE (0x1F) | PSR_FIQ_DIS(0x40) | PSR_IRQ_DIS (0x80)
	mov r0, #0xDF ;@ 001 1111
	msr cpsr_c, r0
	ldr sp, =0x0A827000

	;@ UNDEFINED PSR_UNDEFINED_MODE (0x1B) | PSR_FIQ_DIS (0x40) | PSR_IRQ_DIS (0x80)
	mov r0, #0xDB
	msr cpsr_c, r0
	ldr sp, =0x1008000
	        
	;@ SVC
    mov r0,#0xD3 ;@ PSR_SVC_MODE (0x13) PSR_FIQ_DIS (0x40) | PSR_IRQ_DIS (0x80)
    msr cpsr_c,r0
    ldr sp, =0xC08000
		
	;@ Clear out bss
		ldr	r4, =_bss_start
		ldr	r9, =_bss_end
		mov	r5, #0
		mov	r6, #0
		mov	r7, #0
		mov	r8, #0
			b       2f
	 
		1:
			;@ store multiple at r4.
			stmia	r4!, {r5-r8}
		 
			;@ If we are still below bss_end, loop.
		2:
			cmp	r4, r9
			blo	1b
	;@ ---------------------------------------------
	
	;@ Jump into main function in C (main.c)
	bl cmain

;@ Something has gone wrong, place the CPu in a wait state
hang: 
	b hang

.globl PUT32 ;@ void PUT32(unsigned int address, unsigned int value)
PUT32:
	str r1, [r0]
	bx lr

.globl GET32 ;@ unsigned int GET32(unsigned int address)
GET32:
	ldr r0, [r0]
	bx lr
	
.globl dummy ;@ void dummy(void)
dummy:
	bx lr

.globl get_sp
get_sp:
	mov r0, sp
	bx lr
	
.globl enable_irq ;@ void enable_irq(void)
enable_irq:
    mrs r0, cpsr     ;@ Retrieve status
    bic r0, r0, #0x80 ;@ Clear bit 7 to enable IRQ
    msr cpsr_c, r0   ;@ Write update register back to status register
    bx lr

.globl disable_irq ;@ void disable_irq(void)
disable_irq:
	mrs r0, cpsr      ;@ Retrieve status
	orr r0, r0, #0x80 ;@ Set bit 7 to disable IRQ
	msr cpsr_c, r0    ;@ Write new status back to register
	bx lr
	
.globl enable_fiq ;@ void enable_fiq(void)
enable_fiq:
	mrs r0, cpsr
	bic r0, r0, #0x40 ;@ Clear bit 6 to enable FIQ
	msr cpsr_c, r0
	bx lr
	
.globl disable_fiq ;@ void disable_fiq(void)
disable_fiq:
	mrs r0, cpsr
	orr r0, r0, #0x40
	msr cpsr_c, r0
	bx lr

.globl enable_mmu_and_cache ;@ enable_mmu_and_cache(unsigned int* pageTableBase)
enable_mmu_and_cache:
	;@ Restrict cache size to 16K (no page colouring)
	mrc p15, 0, r1, c1, c0, 1
	orr r1, #0x40
	mcr p15, 0, r1, c1, c0, 1

	;@ Set all domains to 'client'
	ldr r1, =0x55555555
	mcr p15, 0, r1, c3, c0, 0

	;@ Always use TTBR0
	mov r1, #0
	mcr p15, 0, r1, c2, c0, 2

	;@ Set TTBR0 (Page table walk, inner  cacheable, outer non-cacheable)
	orr r0, #1
	mcr p15, 0, r0, c2, c0, 0
	
	;@ Invalidate datacache and flush prefetch buffer
	mov r1, #0
	mcr p15, 0, r1, c7, c5, 4
	mcr p15, 0, r1, c7, c6, 0
	
	;@ Enable MMU, L1 and instruction cache, L2 cache, write buffer
	;@ Branch prediction and extended page table on
	mov r1, #0
	mrc p15, 0, r1, c1, c0, 0
	ldr r2, =0x0480180D
	orr r1, r2 
	mcr p15, 0, r1, c1, c0, 0

	bx lr

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

data_abort:	
	push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}

	;@ Address where it happened
	subs r0, lr, #8

	;@ Get the error type
	mrc p15, 0, r1, c5, c0, 0
	and r1, r1, #0xF
	
	;@ Get address that was accessed
	mrc p15, 0, r2, c6, c0, 0
	bl c_abort_data_handler
	
	pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
	
	subs PC, lr, #4

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

undefined:
    push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    
	mov r0, lr
    mov r1, r14

	bl c_undefined_handler
	
    pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
	
	subs PC, lr, #4

swi:
	;@ Save registers and LR onto stack
    mov r2, r14
	stmfd sp!, {r0-r12,lr}

	;@ SWI number is stored in top 8 bits the instruction
	ldr r0, [lr, #-4]
	bic r0, r0, #0xFF000000

	mov r1, sp

	bl c_swi_handler

	;@ Restore registers and return
	LDMFD sp!,{r0-r12,pc}^
