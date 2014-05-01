#ifndef ASM_H
#define ASM_H

#define PT_CACHEABLE 1
#define PT_NONCACHEABLE 0

// Value to set in TTBC
#define TTBC_SPLIT_16KB 0
#define TTBC_SPLIT_8KB 1
#define TTBC_SPLIT_4KB 2
#define TTBC_SPLIT_2KB 3
#define TTBC_SPLIT_1KB 4
#define TTBC_SPLIT_512B 5
#define TTBC_SPLIT_256B 6
#define TTBC_SPLIT_128B 7

#define TTBC_PD0_DISABLED (1 << 4)
#define TTBC_PD0_ENABLED (0 << 4)
#define TTBC_PD1_DISABLE (1 << 5)
#define TTBC_PD1_ENABLED (0 << 5)

//
// irq.s
//
extern void enable_irq(void);
extern void disable_irq(void);
extern void enable_fiq(void);
extern void disable_fiq(void);

//
// mmu.s
//

// Sets the translation table base 0 register
// NOTE: If cacheable is set, make sure pt is stored in Inner-write through memory
void set_ttb0(unsigned int* pt, unsigned int cacheable);

// Enables the MMU in the Control register configuration data
extern void enable_mmu();

// Invalidates the data and prefetch cache
extern void invalidate_cache();

// Sets the domain register
extern void set_domain_register(unsigned int x);

// Restricts cache size to 16KB (Disables page coloring)
extern void disable_page_coloring();

// Sets the Translation table base control register
extern void set_ttbc(unsigned int val);

// Gets the value of the current program control register
extern unsigned int get_crcd(void);

// Gets the value of the Translation table base 0 register
extern unsigned int get_ttb0(void);

// Gets the value of the Translation table control register
extern unsigned int get_ttbc(void);

// Gets the value of the domain register 
extern unsigned int get_domain_register(void);

//
// utility.s
//
// Gets the current frame pointer
extern int* get_fp(void);

// Utility function for branching to an arbitrary memory address
extern void branchTo(unsigned int* addr);

#endif