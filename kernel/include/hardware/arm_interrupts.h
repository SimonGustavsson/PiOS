#ifndef ARM_INTERRUPTS_H
#define ARM_INTERRUPTS_H

typedef enum {
	unused_interrupt_source0,
	unused_interrupt_source1,
	unused_interrupt_source2,
	interrupt_source_system_timer,
	unused_interrupt_source4,
	unused_interrupt_source5,
	unused_interrupt_source6,
	unused_interrupt_source7,
	unused_interrupt_source8,
	unused_interrupt_source9,
	unused_interrupt_source10,
	unused_interrupt_source11,
	unused_interrupt_source12,
	unused_interrupt_source13,
	unused_interrupt_source14,
	unused_interrupt_source15,

	unused_interrupt_source16,
	unused_interrupt_source17,
	unused_interrupt_source18,
	unused_interrupt_source19,
	unused_interrupt_source20,
	unused_interrupt_source21,
	unused_interrupt_source22,
	unused_interrupt_source23,
	unused_interrupt_source24,
	unused_interrupt_source25,
	unused_interrupt_source26,
	unused_interrupt_source27,
	unused_interrupt_source28,
	interrupt_source_aux,
	unused_interrupt_source30,
	unused_interrupt_source31,

	unused_interrupt_source32,
	unused_interrupt_source33,
	unused_interrupt_source34,
	unused_interrupt_source35,
	unused_interrupt_source36,
	unused_interrupt_source37,
	unused_interrupt_source38,
	unused_interrupt_source39,
	unused_interrupt_source40,
	unused_interrupt_source41,
	unused_interrupt_source42,
	interrupt_source_i2c_spi_slv,
	unused_interrupt_source44,
	interrupt_source_pwa0,
	interrupt_source_pwa1,
	unused_interrupt_source47,

	interrupt_source_smi,
	interrupt_source_gpio0,
	interrupt_source_gpio1,
	interrupt_source_gpio2,
	interrupt_source_gpio3,
	interrupt_source_i2c,
	interrupt_source_spi,
	interrupt_source_pcm,
	unused_interrupt_source_56,
	interrupt_source_uart,
	unused_interrupt_source_58,
	unused_interrupt_source_59,
	unused_interrupt_source_60,
	unused_interrupt_source_61,
	unused_interrupt_source_62,
	unused_interrupt_source_63,

	interrupt_source_ArmTimer,
	interrupt_source_ArmMailbox,
	interrupt_source_ArmDoorbell0,
	interrupt_source_ArmDoorbell1,
	interrupt_source_Gpu0Halted,
	interrupt_source_Gpu1Halted,
	interrupt_source_IllegalAccessType1,
	interrupt_source_IllegalAccessType0,
} interrupt_source;

void Arm_InterruptInitialize(void);
void Arm_IrqEnable(interrupt_source source);
void Arm_IrqDisableall(void);
interrupt_source Arm_IrqGetPending(void);

void Arm_FiqEnable(interrupt_source source);
void Arm_FiqDisableall(void);

#endif
