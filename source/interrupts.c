#include "interrupts.h"
#define INTERRUPT_MAILBOX_FLAG 0x2

extern void enable_irq(void);

InterruptRegister* gInterruptReg;

unsigned int interrupts_init(void)
{
	gInterruptReg = (InterruptRegister*)(INTERRUPT_BASE);
	
	// Enable interrupts in the interrupt controller
	gInterruptReg->Enable1 = INTERRUPT_MAILBOX_FLAG;
	
	// Tell the CPU to start accepting irq's from the peripherals
	enable_irq();
	
	return 0;
}
