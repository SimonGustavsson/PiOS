;@
;@ This file contains functions to set up page tables and enable the MMU
;@

;@
;@ Sets TTB0
;@ C Signature: void set_ttb0(unsigned int* pt, unsigned int cacheable)
;@              pt: Physical address of the page table to install into ttb0
;@              cacheable: Whether the memory is cacheable. 1 = Cacheable, 0 = Noncacheable
.globl set_ttb0
set_ttb0:
    ;@ Add Inner cacheable flag to address
    orr r0, r1

    ;@ Set TTB0: (using defaults: No outer cacheable PT walks, not shared)
    mcr p15, 0, r0, c2, c0, 0

    bx lr
    
;@
;@ Update TTBC
;@ C Signature: void set_ttbc(unsigned int val)
;@
.globl set_ttbc
set_ttbc:
    ;@ TODO: Error checking?
	mcr p15, 0, r0, c2, c0, 2

    bx lr

;@
;@ Restricts cache size to 16K (No page coloring)
;@ C Signature: void disable_page_coloring(void)
;@
.globl disable_page_coloring
disable_page_coloring:
  
    mov r0, #0  
    mrc p15, 0, r0, c1, c0, 1
	orr r1, #0x40
	mcr p15, 0, r0, c1, c0, 1

    bx lr

;@
;@ Sets the domain register
;@ C Signature: void set_domain_register(unsigned int val)
;@              val: The value to set
;@
.globl set_domain_register
set_domain_register:
    ldr r0, =0x55555555
    mcr p15, 0, r0, c3, c0, 0
    
    bx lr
    
;@
;@ Invalidate data cache and prefetch buffer
;@ C Signature: void invalidate_cache(void)
;@
.globl invalidate_cache
invalidate_cache:
    mov r1, #0
	mcr p15, 0, r1, c7, c5, 4
	mcr p15, 0, r1, c7, c6, 0
    
    bx lr

;@
;@ Enable mmu
;@ C Signature: void enable_mmu(void)
;@
.globl enable_mmu
enable_mmu:
	;@ Enable MMU, L1 and instruction cache, L2 cache, write buffer
	;@ Branch prediction and extended page table on
	mov r1, #0
	mrc p15, 0, r1, c1, c0, 0 ;@ Control register configuration data
	ldr r2, =0x0480180D
	orr r1, r2 
	mcr p15, 0, r1, c1, c0, 0    

    bx lr

;@
;@ Enable the MMU and L2
;@ C Signature: void enable_mmu_and_cache(unsigned int* pt)
;@              pt: A pointer to the page table to use
;@
.globl enable_mmu_and_cache
enable_mmu_and_cache:
    push {fp, lr}
    add	fp, sp, #4
    
    blx set_ttb0

    ;@ Save pt as we might trash some registers in all this calling jazz!
    ;@mov r5, r0
    
    ;@blx disable_page_coloring
    ;@ stff
    ;@ldr r0, =0x55555555
    ;@blx set_domain_register
    
    ;@mov r2, #0 ;@ No split, just use ttb0
    ;@blx set_ttbc

    ;@blx invalidate_datacache_prefetch

    ;@blx enable_mmu

    pop {fp, pc}

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
;@ Gets the value of the Translation table base 0 register
;@ C Signature: unsigned int get_ttb0(void)
;@
.globl get_ttb0
get_ttb0:
    mrc p15, 0, r0, c2, c0, 0
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
