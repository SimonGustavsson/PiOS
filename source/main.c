#include "gpio.h"
#include "myfont.h"
#include "timer.h"
#include "framebuffer.h"

void OnCriticalError(void)
{
	// At this point we can't even render text on screen, so just flash the LED
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
					
	while(1)
	{
	}
}
