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
	print(message, length);
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
	else
	{
		if((result = KeyboardInitialise()) != 0)
			printf("Keyboard initialise failed, error code: %d\n", result);
		else
			print("Keyboard initialise success!\n", strlen("Keyboard initialise success!\n"));
	}

	if(result != 0)
		goto halt;
		
		
	while(1)
	{
		KeyboardUpdate();
		
		short scanCode = KeyboardGetChar();		
		
		// Nothing pressed
		if(scanCode == 0)
		{
			wait(10);
			continue;
		}
		
		virtualkey vk = ScanToVirtual(scanCode);
		
		char c = VirtualToAsci(vk);
		
		printf("%c", c);
		// char name[15];
		// char* keyname = GetKeyName(name, 15, vk);
		
		// printf("Scan: %d Vk: %d Name: %s\n", scanCode, (unsigned int)vk, keyname);
		
		wait(10);
	}	
		
halt:	
	print("\nHalting...\n", 12); 
	while(1);
}
