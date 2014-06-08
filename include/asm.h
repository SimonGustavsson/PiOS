#ifndef ASM_H
#define ASM_H

#define PT_CACHEABLE 1
#define PT_NONCACHEABLE 0

// Value to set in TTBC
#define TTBC_SPLIT_16KB 0 // 4 GB TTB0
#define TTBC_SPLIT_8KB  1 // 2 GB TTB0
#define TTBC_SPLIT_4KB  2 // 1 GB TTB0
#define TTBC_SPLIT_2KB  3 // 512 MB TTB0
#define TTBC_SPLIT_1KB  4 // 256 MB TTB0
#define TTBC_SPLIT_512B 5 // 128 MB TTB0
#define TTBC_SPLIT_256B 6 // 64 MB TTB0
#define TTBC_SPLIT_128B 7 // 32 MB TTB0

#define TTBC_PD0_DISABLED (1 << 4)
#define TTBC_PD0_ENABLED (0 << 4)
#define TTBC_PD1_DISABLE (1 << 5)
#define TTBC_PD1_ENABLED (0 << 5)

// Client - Memory access is controlled by TLB entry
#define DAC_CLIENT 0x55555555

// Client - No access (Reset value)
#define DAC_NONE 0x00000000

// Reserved - Any access generates domain fault
#define DAC_RESERVED 0xAAAAAAAA

// Manager - TLB entry is not checked for access, no domain faults are generated
#define DAC_MANAGER 0xFFFFFFFF

//
// interrupt vector
//
extern void irq(void);
extern void fiq(void);
extern void reset(void);
extern void data_abort(void);
extern void instruction_abort(void);
extern void reset(void);

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

// Installs the page tables and enables the MMU
extern void do_mmu(unsigned int* ttb0, unsigned int* ttb1, unsigned int split);

// Sets the translation table base 0 register
// NOTE: If cacheable is set, make sure pt is stored in Inner-write through memory
void set_ttb0(unsigned int* pt, unsigned int cacheable);

// Gets the value of the current program control register
extern unsigned int get_crcd(void);

// Gets the value of the Translation Table 0 base register
extern unsigned int get_ttb0(void);

// Gets the value of the Translation Table 1 Base register
extern unsigned int get_ttb1(void);

// Gets the value of the Translation Table Control register
extern unsigned int get_ttbc(void);

// Gets the value of the domain register 
extern unsigned int get_domain_register(void);

extern unsigned int set_domain_register(unsigned int value);

//
// utility.s
//
// Gets the current frame pointer
extern int* get_fp(void);

// Utility function for branching to an arbitrary memory address
extern void call(unsigned int addr);

// Unconditionally branches to the given address
// WARNING: this trashes FP and does not set up LR!
extern void branch(unsigned int* addr);

#endif