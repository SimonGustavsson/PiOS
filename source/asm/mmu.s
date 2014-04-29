;@
;@ This file contains functions to set up page tables and enable the MMU
;@

;@ Enable the MMU and L2
;@ C Signature: void enable_mmu_and_cache(unsigned int* pt)
;@              pt: A pointer to the page table to use
;@
.globl enable_mmu_and_cache
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
