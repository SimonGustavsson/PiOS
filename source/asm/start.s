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
	ldr sp, =0x01008000
	        
	;@ SVC
    mov r0,#0xD3 ;@ PSR_SVC_MODE (0x13) PSR_FIQ_DIS (0x40) | PSR_IRQ_DIS (0x80)
    msr cpsr_c,r0
    ldr sp, =0xC08000
	
	;@ Jump into main function in C (main.c)
	bl cmain

;@ Something has gone wrong, place the CPu in a wait state
hang: 
	b hang

;@ Gets the current Frame Pointer, used for stack trace
.align 2
.globl get_frame_pointer ;@ int* get_frame_pointer(void)
get_frame_pointer:
    mov r0, fp
    mov pc, lr
	
.globl branchTo ;@ branchTo(unsigned int* addr)
branchTo:
    push {fp, lr}
    add	fp, sp, #4

    mov r2, r0
    blx r2
    pop {fp, lr}
    
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
