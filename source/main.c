#include "gpio.h"
#include "timer.h"
#include "terminal.h"
#include "stringutil.h"
#include "usbd/usbd.h"
#include "keyboard.h"

void OnCriticalError(void)
{
	while(1)
	{
		LedOff();
		
		wait(1000);		
		
		LedOn();		
		
		wait(1000);
	}
}

// Log function for CSUD
void LogPrint(char* message, unsigned int length)
{
	// Disabled usb driver output for now, it's not interesting enough!
	//print(message, length);
}

int cmain(void)
{
	unsigned int result = 0;
	
	LedInit();
	
	if((result = terminal_init()) != 0)
	{
		OnCriticalError(); // Critical error: Failed to initialize framebuffer :-(
	}

	if((result = UsbInitialise()) != 0)
		printf("Usb initialise failed, error code: %d\n", result);
	else if((result = KeyboardInitialise()) != 0)
			printf("Keyboard initialise failed, error code: %d\n", result);
	
	print("Welcome to PiOS!\nPiOS->", strlen("Welcome to PiOS!\nPiOS->"));
	
	if(result != 0)
		goto halt;
				
	while(1)
	{
		terminal_update();
		
		wait(10);
	}	
		
halt:	
	print("\nHalting...\n", 12); 
	while(1);
}
