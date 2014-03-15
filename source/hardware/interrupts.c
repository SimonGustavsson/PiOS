#include "hardware/interrupts.h"

static ArmInterrupts* gInterrupts;

void arm_interrupt_init(void)
{
	gInterrupts = (ArmInterrupts*)(INTERRUPT_BASE);
}

void arm_irq_enable(interrupt_source source)
{
	if (source < 32)
		gInterrupts->irq_enable1.raw |= (1 << source);
	else
		gInterrupts->irq_enable2.raw |= (1 << (source - 32));
}

void arm_irq_disableall(void)
{
	gInterrupts->irq_disable1.raw = 0xFFFFFFFF;
	gInterrupts->irq_disable2.raw = 0xFFFFFFFF;
}

interrupt_source arm_irq_getPending(void)
{
	if (gInterrupts->irq_pending1.bits.system_timer)
		return interrupt_source_system_timer;
	else if (gInterrupts->irq_pending2.bits.uart)
		return interrupt_source_uart;

	// Unknown
	return gInterrupts->irq_pending1.raw;
}

void arm_fiq_enable(interrupt_source source)
{
	fiq_control_req r = gInterrupts->fiq_control;

	r.bits.enable = 1;
	r.bits.source = source;

	gInterrupts->fiq_control.raw = r.raw;
}

void arm_fiq_disable(void)
{
	gInterrupts->fiq_control.raw = 0;
}
