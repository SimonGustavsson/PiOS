#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "hardware/arm_interrupts.h"

// Note: Read-only
typedef union {
	unsigned int raw;

	struct {
		int arm_timer : 1;
		int arm_mailbox : 1;
		int arm_doorbell0 : 1;
		int arm_doorbell1 : 1;
		int gpu0_halted : 1;
		int gpu1_halted : 1;
		int illegal_access_type1 : 1;
		int illegal_access_type0 : 1;
		int pending_reg_1_set : 1;
		int pending_reg_2_set : 1;
		int gpu_irq7 : 1;
		int gpu_irq9 : 1;
		int gpu_irq10 : 1;
		int gpu_irq_18 : 1;
		int gpu_irq_19 : 1;
		int gpu_irq_53 : 1;
		int gpu_irq_54 : 1;
		int gpu_irq_55 : 1;
		int gpu_irq_56 : 1;
		int gpu_irq_57 : 1;
		int gpu_irq_62 : 1;
		int reserved : 11;
	} bits;
} basic_irq_pending_req;

typedef union {
	unsigned int raw;

	struct {
		unsigned int reserved0 : 1;
		unsigned int reserved1 : 1;
		unsigned int reserved2 : 1;
		unsigned int system_timer : 1;
		unsigned int reserved4 : 1;
		unsigned int reserved5 : 1;
		unsigned int reserved6 : 1;
		unsigned int reserved7 : 1;
		unsigned int reserved8 : 1;
		unsigned int reserved9 : 1;
		unsigned int reserved10 : 1;
		unsigned int reserved11 : 1;
		unsigned int reserved12 : 1;
		unsigned int reserved13 : 1;
		unsigned int reserved14 : 1;
		unsigned int reserved15 : 1;
		unsigned int reserved16 : 1;
		unsigned int reserved17 : 1;
		unsigned int reserved18 : 1;
		unsigned int reserved19 : 1;
		unsigned int reserved20 : 1;
		unsigned int reserved21 : 1;
		unsigned int reserved22 : 1;
		unsigned int reserved23 : 1;
		unsigned int reserved24 : 1;
		unsigned int reserved25 : 1;
		unsigned int reserved26 : 1;
		unsigned int reserved27 : 1;
		unsigned int reserved28 : 1;
		unsigned int aux : 1;
		unsigned int reserved29 : 1;
		unsigned int reserved30 : 1;
	} bits;
} irq_table1;

typedef union {
	unsigned int raw;

	struct {
		unsigned int reserved0 : 1;
		unsigned int reserved1 : 1;
		unsigned int reserved2 : 1;
		unsigned int reserved3 : 1;
		unsigned int reserved4 : 1;
		unsigned int reserved5 : 1;
		unsigned int reserved6 : 1;
		unsigned int reserved7 : 1;
		unsigned int reserved8 : 1;
		unsigned int reserved9 : 1;
		unsigned int reserved10 : 1;
		unsigned int i2c_spi_slv_int : 1;
		unsigned int reserved11 : 1;
		unsigned int pwa0 : 1;
		unsigned int pwa1 : 1;
		unsigned int reserved12 : 1;
		unsigned int smi : 1;
		unsigned int gpio0 : 1;
		unsigned int gpio1 : 1;
		unsigned int gpio2 : 1;
		unsigned int gpio3 : 1;
		unsigned int i2c : 1;
		unsigned int spi : 1;
		unsigned int pcm : 1;
		unsigned int reserved13 : 1;
		unsigned int uart : 1;
		unsigned int reserved14 : 1;
		unsigned int reserved15 : 1;
		unsigned int reserved16 : 1;
		unsigned int reserved17 : 1;
		unsigned int reserved18 : 1;
		unsigned int reserved19 : 1;
	} bits;
} irq_table2;

typedef union {
	unsigned int raw;

	struct {
		unsigned int source : 7; // see interrupt_source enumeration
		unsigned int enable : 1;
		unsigned int reserved : 24;
	} bits;
} fiq_control_req;

typedef union{
	unsigned int raw;

	struct{
		unsigned int timer : 1;
		unsigned int mailbox : 1;
		unsigned int doorbell0 : 1;
		unsigned int doorbell1 : 1;
		unsigned int gpu0halted : 1;
		unsigned int gpu1halted : 1;
		unsigned int access_error_type0 : 1;
		unsigned int access_error_type1 : 1;
		unsigned int reserved : 24;
	} bits;
} basic_interrupt_reg;

#pragma pack(1)

typedef volatile struct {
	basic_irq_pending_req basic_irq_pending;
	irq_table1 irq_pending1;
	irq_table2 irq_pending2;
	fiq_control_req fiq_control;
	irq_table1 irq_enable1;
	irq_table2 irq_enable2;
	basic_interrupt_reg basic_irq_enable;
	irq_table1 irq_disable1;
	irq_table2 irq_disable2;
	basic_interrupt_reg basic_irq_disable;
} ArmInterrupts;

#pragma pack()

void Arm_InterruptInitialize(void);
void Arm_IrqEnable(interrupt_source source);
void Arm_IrqDisableall(void);
interrupt_source Arm_IrqGetPending(void);

void Arm_FiqEnable(interrupt_source source);
void Arm_FiqDisableall(void);

#endif
