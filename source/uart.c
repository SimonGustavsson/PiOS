#include "uart.h"
#include "gpio.h"
#include "timer.h"

static gpio_reg *gGpio;
static Uart *gUart;

unsigned int uart_initialize(void)
{
	gGpio = (gpio_reg*)GPIO_BASE;
	gUart = (Uart*)UART_BASE;

	// Setup the uart
	gUart->enables.bits.mini_uart = 1;
	gUart->mu_ier.raw = 0;
	gUart->mu_cntl.raw = 0;
	gUart->mu_lcr.bits.data_size = 1; // 8-bit.. !? Set second bit to 1 as well? REserved? huh?
	gUart->mu_mcr.raw = 0;
	gUart->mu_ier.raw = 0;
	gUart->mu_iir.raw = 0xC6; // Why? Only first two is used?
	gUart->baud_rate.bits.baud_rate = 270; // ((250,000,000 / 115200) / 8) + 1;
	
	// Enable Uart on the GPIO pins
	gGpio->gpfsel1.bits.fsel4 |= 2; // Gpio 14 (TXD0 & TXD1)
	gGpio->gppud.raw = 0;

	wait(5);

	// Enable pull down/up clock on pin 14
	gGpio->gppudclk0.bits.pin14 = 1;

	wait(5);

	// Disable all pull down/up clocks
	gGpio->gppudclk0.raw = 0;

	gUart->mu_cntl.bits.receiver_enabled = 1;
	gUart->mu_cntl.bits.transmitter_enabled = 1;

	// Now ready to start sending.
	// Example: Spam random characters
	// unsigned int c = 0;
	// while(1)
	// {
		// while(1)
		// {
			// if(gUart->mu_lsr.bits.transmitter_empty)
				// break;

			// gUart->mu_io.raw = 0x30 + (c ++ & 7);
		// }
	// }
}
