#ifndef ASM_H
#define ASM_H

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
extern void set_ttb0(unsigned int* pt, unsigned int n);

// Enables the MMU in the Control register configuration data
extern void enable_mmu();

// Invalidates the data and prefetch cache
extern void invalidate_cache();

// Sets the domain register
extern void set_domain_register(unsigned int x);

// Restricts cache size to 16KB (Disables page coloring)
extern void disable_page_coloring();

// Sets the Translation table base control register
extern void set_ttbc(unsigned int ttb0, unsigned int ttb1, unsigned int b);

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