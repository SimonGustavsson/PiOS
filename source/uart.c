/* uart.c - UART initialization & communication */
/* Reference material:
* http://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
* Chapter 13: UART
*/

#include <uart.h>
#include "gpio.h"

Uart* gUart;

void uart_init() {

	gUart = (Uart*)(UART_BASE);

	// Disable UART0.
	gUart->cr = 0x00000000;

	// Setup GPIO for uart use
	gpio_enable_uart();

	// Clear pending interrupts.
	gUart->icr = 0x7FF;

	// Set integer & fractional part of baud rate.
	gUart->ibrd = 1;
	gUart->fbrd = 40;

	// 8 bit data transmissio (1 stop bit, no parity). (Note: Not enabling fifo)
	gUart->lcrh = (1 << 5) | (1 << 6);

	// Mask all interrupts.
	gUart->imsc = (1 << 1) | (1 << 4) | (1 << 5) |
		(1 << 6) | (1 << 7) | (1 << 8) |
		(1 << 9) | (1 << 10);

	// Enable UART0, receive & transfer part of UART.
	gUart->cr = (1 << 0) | (1 << 8) | (1 << 9);
}

void uart_irpt_enable(void)
{
	gUart->imsc = 0x0010;
}

void uart_putc(unsigned char byte)
{
	// wait for UART to become ready to transmit
	while (1)
	{
		if (!(gUart->fr & (1 << 5)))
			break;
	}

	gUart->dr = byte;
}

unsigned char uart_getc()
{
	// wait for UART to have recieved something
	while (1)
	{
		if (!(gUart->fr & (1 << 4)))
			break;
	}

	return gUart->dr;
}

void uart_puts(const char *str)
{
	while (*str)
	{
		uart_putc(*str++);
	}
}
