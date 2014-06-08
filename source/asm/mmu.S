;@
;@ This file contains functions to set up page tables and enable the MMU
;@

#include "memory_map.h"

;@ C Signature void do_mmu(uint* ttb0, uint* ttb1, uint split)
.section .text.init
.globl do_mmu
do_mmu:

    ;@ Save the SP so we can restore its relative position 
    ;@ After we've enabled virtual memory
    mov r5, sp
    ldr r6, =SVC_STACK_PA_START
    sub r5, r6, r5

    ;@ Disable page coloring
    mrc p15, 0, r3, c1, c0, 1
	orr r3, #0x40
	mcr p15, 0, r3, c1, c0, 1

    ;@ Setup domains
    ldr r3, =0x55555555
    mcr p15, 0, r3, c3, c0, 0

    ;@ Setup TTBC
    mcr p15, 0, r2, c2, c0, 2

    ;@ Setup TTB0 (always cacheable)
    orr r0, #1
    mcr p15, 0, r0, c2, c0, 0

    ;@ Setup TTB1 (always cacheable)
    orr r1, #1
    mcr p15, 0, r1, c2, c0, 1
    
    ;@ Flush prefetch buffer
    mov r1, #0
	mcr p15, 0, r1, c7, c5, 4
    ;@ Invalidate Entire Data Cache
	mcr p15, 0, r1, c7, c6, 0

    ;@ Enable MMU
    mov r1, #0
    mrc p15, 0, r1, c1, c0, 0 ;@ Control register configuration data
    ldr r2, =0x0480180D
    orr r1, r2 
    mcr p15, 0, r1, c1, c0, 0

    ;@ Now that the MMU is enabled, we need to modify the location of the stacks

    mov r0,#0xD2 ;@ IRQ
    msr cpsr_c,r0
    ldr sp, =IRQ_STACK_VA_START

    mov r0,#0xD1 ;@ FIQ
    msr cpsr_c,r0
    ldr sp, =FIQ_STACK_VA_START

	mov r0, #0xD7 ;@ Abort
	msr cpsr_c,r0
	ldr sp, =ABORT_STACK_VA_START

	mov r0, #0xDF ;@ System
	msr cpsr_c, r0
	ldr sp, =SM_STACK_VA_START

	mov r0, #0xDB ;@ Undefined
	msr cpsr_c, r0
	ldr sp, =UD_STACK_VA_START
	        
    add r1, #KERNEL_VA_START
    mov r0,#0xD3 ;@ SVC
    msr cpsr_c,r0

    ;@ Restore the saved relative stack pointer
    ldr r0, =SVC_STACK_VA_START
    sub sp, r0, r5
    
    bx lr
    
;@
;@ Sets TTB0
;@ C Signature: void set_ttb0(unsigned int* pt, unsigned int cacheable)
;@              pt: Physical address of the page table to install into ttb0
;@              cacheable: Whether the memory is cacheable. 1 = Cacheable, 0 = Noncacheable
.section .text
.globl set_ttb0
set_ttb0:
    ;@ Add Inner cacheable flag to address
    orr r0, r1

    ;@ Set TTB0: (using defaults: No outer cacheable PT walks, not shared)
    mcr p15, 0, r0, c2, c0, 0

    ;@ Flush prefetch buffer
    mov r1, #0
	mcr p15, 0, r1, c7, c5, 4
    ;@ Invalidate Entire Data Cache
	mcr p15, 0, r1, c7, c6, 0

    bx lr
    
;@
;@ Gets the value of the Control register configuration data
;@ C Signature: unsigned int get_crcd(void)
;@
.globl get_crcd
get_crcd:
    mrc p15, 0, r0, c1, c0, 0
    bx lr
    
;@ 
;@ Gets the value of the Translation table 0 base register
;@ C Signature: unsigned int get_ttb0(void)
;@
.globl get_ttb0
get_ttb0:
    mrc p15, 0, r0, c2, c0, 0
    bx lr
    
;@
;@ Gets the value of the Translation Table 1 Base Register
;@ C Signature: unsigned int get_ttb1(void)
;@
.globl get_ttb1
get_ttb1:
    mrc p15, 0, r0, c2, c0, 1
    bx lr

;@
;@ Gets the value of the Translation table base control register
;@ C Signature: unsigned int get_ttbc(void)
;@
.globl get_ttbc
get_ttbc:
	mrc p15, 0, r0, c2, c0, 2    

    bx lr

;@
;@ Sets the value of the Domain Access Control Register
;@ C Signature: void set_domain_register(unsigned int)
;@
.globl set_domain_register
set_domain_register:
    MCR p15, 0, r0, c3, c0, 0

    bx lr

;@
;@ Gets the value of the domain register
;@ C Signature: unsigned int get_domain_register(void)
;@
.globl get_domain_register
get_domain_register:
    mrc p15, 0, r0, c3, c0, 0

    bx lr

;@ =====================================================================================================================
    
;@ 
;@ Reference documentation
;@ 
;@ Translation table base control register:
;@ [31:6]    UNP/SBZ
;@ [5]       PD1      - Specifies whether to perform PT walk on TTB1 on TLB miss
;@               0 = Enabled (Reset value)
;@               1 = Disabled
;@ [4]       PD0      - Specifies whether to perform PT walk on TTB0 on TLB miss
;@ [3]       UNO/SBZ
;@ [2-0]     N        - Specifies boundry of Translation table base register 0
;@               000 = 16 KB    - 4 GB   of memory (Reset value)
;@               001 = 8 KB     - 2 GB   of memory
;@               010 = 4 KB     - 1 GB   of memory
;@               011 = 2 KB     - 512 MB of memory
;@               100 = 1 KB     - 256 MB of memory
;@               101 = 512 Byte - 128 MB of memory
;@               110 = 256 Byte - 64 MB  of memory
;@               111 = 128-Byte - 32 MB  of memory
;@
;@  Translation table register:
;@ [31:14-n] Translation table base
;@ [13-n:5]  UNP/SBZ
;@ [4:3]     Outer cachable for page table walks.
;@              00 = No Cacheable (Reset value)
;@              01 = Write back, Write allocate
;@              10 = Write-through, No allocate on write
;@              11 = Write back, no allocate on write
;@ [2]       SBZ (ECC not supported on ARM1176ZF-S)
;@ [1]       Shared
;@               0 = Not shared (Reset value)
;@               1 = Shared
;@ [0]       Inner cacheable
;@               0 = Inner noncacheable (Reset value)
;@               1 = Inner cacheable
