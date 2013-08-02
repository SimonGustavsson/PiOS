extern unsigned int GET32(unsigned int);
extern void PUT32(unsigned int, unsigned int);

#include "gpio.h"
#include "myfont.h"
#include "timer.h"
#include "framebuffer.h"

void OnCriticalError(void)
{
	while(1)
	{
		LedOff();
		Wait(1);		
		LedOn();		
		Wait(1);
	}
}

int cmain(void)
{
	LedInit();
	
	if(InitializeFramebuffer() != 0)
	{
		OnCriticalError(); // Critical error: Failed to initialize framebuffer :-(
		while(1);
	}
	
	
	Write("Hello, World!");
					
	// unsigned int i;
	// for(i = 25; i < 100; i++)
	// {
		// unsigned int j;
		// for(j = 25; j < 100; j++)
		// {
			// DrawPixel(i, j, 0xF1F1);
		// }
	// }	
	
	while(1)
	{
	}
}
