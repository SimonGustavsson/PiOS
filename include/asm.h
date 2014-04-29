#ifndef ASM_H
#define ASM_H

// irq.s
extern void enable_irq(void);
extern void disable_irq(void);
extern void enable_fiq(void);
extern void disable_fiq(void);

// mmu.s
extern void enable_mmu_and_cache(unsigned int* pt);

// utility.s
extern int* get_frame_pointer(void);
extern void branchTo(unsigned int* addr);

#endif