#include "gpio.h"

static gpio_reg* gGpio;

unsigned int gpio_initialize(void)
{
	gGpio = (gpio_reg*)GPIO_BASE;
	
	// Enable output on the LED
	unsigned int ra = gGpio->gpfsel1.raw;
	
	ra &= ~(7 << 18);
	ra |= 1 << 18;
	
	gGpio->gpfsel1.raw = ra;

	return 0;
}

void LedOn()
{
	gGpio->gpclr0.raw = (1 << 16);
}

void LedOff()
{
	gGpio->gpset0.raw = (1 << 16);
}
