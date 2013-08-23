#include "gpio.h"
#include "timer.h"
#include "terminal.h"
#include "stringutil.h"
#include "usbd/usbd.h"
#include "stdio.h"

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
	print(message, length);
}

int cmain(void)
{
	LedInit();
		
	if(terminal_init() != 0)
	{
		OnCriticalError(); // Critical error: Failed to initialize framebuffer :-(
	}

	stdio_init();
	
	print("Terminal initialized", strlen("Terminal initialized"));
	print("Initialising USB", strlen("Initialising USB"));

	UsbInitialise();

	print("USB Initialisation done.\n", strlen("USB Initialisation done.\n"));

	print("Write something! ", strlen("Write something! "));

	char inputBuf[256];

	cin(inputBuf, 256);

	print(inputBuf, 256);

	print("\nHalting...\n", 12); 
	while(1);
}
