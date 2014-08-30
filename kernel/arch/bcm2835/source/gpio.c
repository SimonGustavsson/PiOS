#include "hardware/gpio.h"
#include "memory_map.h"

static void delay(unsigned int count)
{
	volatile unsigned int i;
	for (i = 0; i < count; i++){ /* Do Nothing */ }
}

void Gpio_EnableUart(void)
{
    gpio_reg* gpio = (gpio_reg*)(PERIPHERAL_VA_GPIO);

	// Disable pull up/down for all GPIO pins & delay for 150 cycles.
	gpio->gppud.raw = 0x00000000;
	delay(150);

	// Disable pull up/down for pin 14,15 & delay for 150 cycles.
	gpio->gppudclk0.raw = (1 << 14) | (1 << 15);
	delay(150);

	// Write 0 to GPPUDCLK0 to make it take effect.
	gpio->gppudclk0.raw = 0x00000000;
}
