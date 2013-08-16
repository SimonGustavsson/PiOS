#include "gpio.h"
#include "myfont.h"
#include "timer.h"
#include "framebuffer.h"
// TODO: Finish terminal.c and use instead of framebuffer in main
#include "terminal.h"
#include "stringutil.h"

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
		
	if(terminal_init() != 0)
	{
		OnCriticalError(); // Critical error: Failed to initialize framebuffer :-(
	}

										
	char* greeting = "Hello, Terminal World! This is your terminal speaking. Please remember to termina all commands with the enter-key, and smile whilst doing so. Failure to comply will result in termination.";
	unsigned int maxLength = strlen(greeting);
	
	unsigned int counter = 0;
	while(1)
	{
		print(greeting, counter);
		print("\n", 1);
		
		if(counter < maxLength)
			counter++;
		else
			counter = 0;
		
		Wait(1); // Wait for quarter of a second
	}
}
