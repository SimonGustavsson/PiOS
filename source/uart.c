#include "uart.h"
#include "gpio.h"

static Uart* gUart;

unsigned int uart_initialize(void)
{
	gpio_reg* gpio = (gpio_reg*)(GPIO_BASE);
	gUart = (Uart*)(UART_BASE);

	// Disable uart
	gUart->cr = 0;

	// Setup GPIO pin 14 & 15
	
	// Disable pull up/down for all GPIO pins
	gpio->gppud.raw = 0;
	volatile unsigned int i;
	for(i = 0; i < 150; i++) { /* Do Nothing */ } 

	// Disable pull up/down for pin 14 and 15
	gpio->gppudclk0.raw = ((1 << 14) | (1 << 15));
	for(i = 0; i < 150; i++) { /* Do Nothing */ } 

	// Write 0 to make it take effect
	gpio->gppudclk0.raw = 0;

	// Clear pending interrupts
	gUart->icr = 0x7FF;

    // Set integer & fractional part of baud rate.
    // Divider = UART_CLOCK/(16 * Baud)
    // Fraction part register = (Fractional part * 64) + 0.5
    // UART_CLOCK = 3000000; Baud = 115200.
 
    // Divider = 3000000/(16 * 115200) = 1.627 = ~1.
    // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
	gUart->ibrd = 1;
	gUart->fbrd = 40;

	// Enable FIFO and 8 bit transmission (1 stop bit, no parity)
	gUart->lcrh = ((1 << 4) | (1 << 5) | (1 << 6));

	// Mask all interrupts
	gUart->imsc = ((1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

	// Enable receiving and transmission
	gUart->cr = ((1 << 0) | (1 << 8) | (1 << 9));

	return 0;
}

void uart_send(unsigned int c)
{
	while(1)
	{
		// Wait for the uart to become ready for transmission
		if(!(gUart->fr & (1 << 5)))
			break;
	}

	gUart->dr = c;
}

unsigned int uart_read()
{
	while(1)
	{
		// Wait for the uart to receive some data
		if(!(gUart->fr & (1 << 4)))
			break;
	}
	
	return gUart->dr;
}

void uart_send_string(char* s)
{
	while(*s != '\0')
	{
		uart_send(*s);

		s++;
	}
}
