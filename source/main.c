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
		
		
	printf("NUM_DEL is %d", (int)NUM_DEL);
		
	while(1)
	{
		KeyboardUpdate();
		short scanCode = KeyboardGetChar();		
		
		if(scanCode != 0)
		{
			virtualkey vk = ScanToVirtual(scanCode);
			
			char c = VirtualToAsci(vk);
			if(c >0)
				printf("%c", c);
			else if(c == -1)
				printf("\nsc:%d vk:%d unmapped in asci table.", (int)scanCode, (int)vk, (int)c);
			else
			{
				switch(vk)
				{
					case VK_BSP:
						terminal_back();
						break;
					case VK_ENTER:
						print("\n", 1);
						break;
					default:
						printf("\nsc:%d vk:%d ch:%d", (int)scanCode, (int)vk, (int)c);
						break;
				}
			}
		}
		wait(10);
	}	
		
halt:	
	print("\nHalting...\n", 12); 
	while(1);
}
