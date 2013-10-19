#include "gpio.h"
#include "stringutil.h"

extern unsigned int GET32(unsigned int);
extern void PUT32(unsigned int, unsigned int);

#define GPFSEL1 0x20200004
#define GPSET0	 0x2020001C
#define GPCLR0	 0x20200028

unsigned int gpio_initialize(void)
{
	//gpio_reg* gpio = (gpio_reg*)GPIO_BASE;
	/*printf("GPIO - gpfsel0: 0x%h.\n", &gpio->gpfsel0.raw);
	printf("GPIO - gpfsel1: 0x%h.\n", &gpio->gpfsel1.raw);
	printf("GPIO - gpfsel2: 0x%h.\n", &gpio->gpfsel2.raw);
	printf("GPIO - gpfsel3: 0x%h.\n", &gpio->gpfsel3.raw);	
	printf("GPIO - gpfsel4: 0x%h.\n", &gpio->gpfsel4.raw);	
	printf("GPIO - gpfsel5: 0x%h.\n", &gpio->gpfsel5.raw);	
	printf("GPIO - reserved: 0x%h.\n", &gpio->reserved);
	printf("GPIO - gpset0: 0x%h.\n", &gpio->gpset0.raw);
	printf("GPIO - gpset1: 0x%h.\n", &gpio->gpset1.raw);
	printf("GPIO - reserved2: 0x%h.\n", &gpio->reserved2);
	printf("GPIO - gpclr0: 0x%h.\n", &gpio->gpclr0.raw);
	printf("GPIO - gpclr1: 0x%h.\n", &gpio->gpclr1.raw);
	printf("GPIO - reserved3: 0x%h.\n", &gpio->reserved3);
	printf("GPIO - gplev0: 0x%h.\n", &gpio->gplev0.raw);
	printf("GPIO - gplev1: 0x%h.\n", &gpio->gplev1.raw);
	printf("GPIO - reserved4: 0x%h.\n", &gpio->reserved4);
	printf("GPIO - gpeds0: 0x%h.\n", &gpio->gpeds0.raw);
	printf("GPIO - gpeds1: 0x%h.\n", &gpio->gpeds1.raw);	
	printf("GPIO - reserved5: 0x%h.\n", &gpio->reserved5);
	printf("GPIO - gpren0: 0x%h.\n", &gpio->gpren0.raw);
	printf("GPIO - gpren1: 0x%h.\n", &gpio->gpren1.raw);
	printf("GPIO - reserved6: 0x%h.\n", &gpio->reserved6);
	printf("GPIO - gpfen0: 0x%h.\n", &gpio->gpfen0.raw);
	printf("GPIO - gpfen1: 0x%h.\n", &gpio->gpfen1.raw);
	printf("GPIO - reserved7: 0x%h.\n", &gpio->reserved7);
	printf("GPIO - gphen0: 0x%h.\n", &gpio->gphen0.raw);
	printf("GPIO - gphen1: 0x%h.\n", &gpio->gphen1.raw);
	printf("GPIO - reserved8: 0x%h.\n", &gpio->reserved8);
	printf("GPIO - gplen0: 0x%h.\n", &gpio->gplen0.raw);
	printf("GPIO - gplen1: 0x%h.\n", &gpio->gplen1.raw);
	printf("GPIO - reserved9: 0x%h.\n", &gpio->reserved9);
	printf("GPIO - gparen0: 0x%h.\n", &gpio->gparen0.raw);
	printf("GPIO - gparen1: 0x%h.\n", &gpio->gparen1.raw);
	printf("GPIO - reserved10: 0x%h.\n", &gpio->reserved10);	
	printf("GPIO - gpafen0: 0x%h.\n", &gpio->gpafen0.raw);
	printf("GPIO - gpafen1: 0x%h.\n", &gpio->gpafen1.raw);
	printf("GPIO - reserved11: 0x%h.\n", &gpio->reserved11);
	printf("GPIO - gppud: 0x%h.\n", &gpio->gppud.raw);
	printf("GPIO - gppudclk0: 0x%h.\n", &gpio->gppudclk0.raw);
	printf("GPIO - gppudclk1: 0x%h.\n", &gpio->gppudclk1.raw);
	printf("GPIO - test: 0x%h.\n", &gpio->test);*/

	// Enable output on the LED
	unsigned int ra = GET32(GPFSEL1); 
	
	ra &= ~(7 << 18);
	ra |= 1 << 18;
	
	PUT32(GPFSEL1, ra);

	return 0;
}

void LedOn()
{
	PUT32(GPCLR0, 1 << 16); // On
}

void LedOff()
{
	PUT32(GPSET0, 1 << 16); // Off
}
