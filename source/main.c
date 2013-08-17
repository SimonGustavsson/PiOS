#include "gpio.h"
#include "myfont.h"
#include "timer.h"
#include "framebuffer.h"
// TODO: Finish terminal.c and use instead of framebuffer in main
#include "terminal.h"
#include "stringutil.h"
#include "usbd/usbd.h"

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
	
	UsbInitialise();

	char buffer[10];	
	printf("&buffer=%d", (int)&buffer);
		
	print("\nHalting...\n", 12); 
	while(1);
}
