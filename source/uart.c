#include "uart.h"
#include "gpio.h"
#include "timer.h"
#include "stringutil.h"

//static gpio_reg *gGpio;
static Uart *gUart;
#define GPFSEL1 0x20200004
#define GPPUD       0x20200094
#define GPPUDCLK0   0x20200098

unsigned int uart_initialize(void)
{
	//gGpio = (gpio_reg*)GPIO_BASE;
	gUart = (Uart*)UART_BASE;

	// Setup the uart
	gUart->enables.bits.mini_uart = 1;
	gUart->mu_ier.raw = 0;
	gUart->mu_cntl.raw = 0;
	gUart->mu_lcr.bits.data_size = 3; // 8-bit.. !? Set second bit to 1 as well? REserved? huh?
	gUart->mu_mcr.raw = 0;
	gUart->mu_ier.raw = 0;
	gUart->mu_iir.raw = 0xC6; // Why? Only first two is used?
	gUart->baud_rate.bits.baud_rate = 270; // ((250,000,000 / 115200) / 8) + 1;

	printf("uart - enables: %d.\n", gUart->enables.raw);
	printf("uart - mu_lcr: %d.\n", gUart->mu_lcr.raw);
	printf("uart - baud_rate: %d.\n", gUart->baud_rate.raw);
	
	// Enable Uart on the GPIO pins
	//gGpio->gpfsel1.bits.fsel4 |= 2; // Gpio 14 (TXD0 & TXD1)
	//gGpio->gppud.raw = 0;
	*(volatile unsigned int*)GPPUD = 0;

	printf("GPFSEL1: %d\n", *(unsigned int*)GPFSEL1);
	volatile unsigned int var =  *(unsigned int*)GPFSEL1;
	var &= ~(7 << 12); //gpio14
    var |= 2 << 12;    //alt5

	*(unsigned int*)GPFSEL1 = var;

	wait(5);

	// Enable pull down/up clock on pin 14
	//gGpio->gppudclk0.bits.pin14 = 1;
	printf("GPPUDCLK0 = %d.\n", *(volatile unsigned int*)GPPUDCLK0);
	*(volatile unsigned int*)GPPUDCLK0 = (1 << 14);

	wait(5);
	
	*(volatile unsigned int*)GPPUDCLK0 = 0;

	// Disable all pull down/up clocks
	//gGpio->gppudclk0.raw = 0;

	gUart->mu_cntl.bits.receiver_enabled = 1;
	gUart->mu_cntl.bits.transmitter_enabled = 1;
	return 0;
}

void uart_send_string(char* s)
{
	while(*s != '\0')
	{
		uart_send_char(*s);

		s++;
	}
}

void uart_send_char(unsigned int c)
{
	while(1)
	{
		if(gUart->mu_lsr.raw & 0x20) // Transmitter empty
			break;
	}

	if(c == '\n')
	{
		gUart->mu_io.raw = '\n'; // LF

		uart_send_char('\r'); // Carriage return
	}
	else
	{
		gUart->mu_io.raw = c;
	}
}
