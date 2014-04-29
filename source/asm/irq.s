;@
;@ This file contains utility functions for enabling/disabling IRQ/FIQ in the CPU
;@

;@
;@ Updates CPSR to enable IRQ interrupts
;@ C Signature: void enable_irq(void)
;@
.globl enable_irq ;@ 
enable_irq:
    mrs r0, cpsr     ;@ Retrieve status
    bic r0, r0, #0x80 ;@ Clear bit 7 to enable IRQ
    msr cpsr_c, r0   ;@ Write update register back to status register
    bx lr

;@
;@ Updates CPSR to disable IRQ interrupts
;@ C Signature: void disable_irq(void)
;@
.globl disable_irq
disable_irq:
	mrs r0, cpsr      ;@ Retrieve status
	orr r0, r0, #0x80 ;@ Set bit 7 to disable IRQ
	msr cpsr_c, r0    ;@ Write new status back to register
	bx lr
	
;@
;@ Updates CPSR to enable FIQ interrupts
;@ C Signature: void enable_fiq(void)
;@
.globl enable_fiq
enable_fiq:
	mrs r0, cpsr
	bic r0, r0, #0x40 ;@ Clear bit 6 to enable FIQ
	msr cpsr_c, r0
	bx lr
	
;@
;@ Updates CPSR to disable FIQ interrupts
;@ C Signature: void disable_fiq(void)
;@
.globl disable_fiq
disable_fiq:
	mrs r0, cpsr
	orr r0, r0, #0x40
	msr cpsr_c, r0
	bx lr
