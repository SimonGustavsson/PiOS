#include "emmc.h"
#include "gpio.h"
#include "interrupts.h"
#include "keyboard.h"
#include "stringutil.h"
#include "terminal.h"
#include "timer.h"
#include "usbd/usbd.h"

volatile unsigned int irq_counter;
timer* gTimer = (timer*)TIMER_BASE;
const unsigned int TIMER_INTERRUPT_INTERVAL = 0x00200000;

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

void c_irq_handler (void)
{
	irq_counter++;
	
	// TODO: Determine source of IRQ - for now this will always be the timer (?)
	printf("IRQ no. %d\n", irq_counter);
	
	// Reset interval
	timer_clearmatch(gTimer, 1);
	timer_setinterval(gTimer, 1, TIMER_INTERRUPT_INTERVAL);
}

int cmain(void)
{
	unsigned int result = 0;
	irq_counter = 0;
	
	LedInit();
	
	// Initialize terminal before enabling interrupts as the interrupt handlers currently try to write to it
	if((result = terminal_init()) != 0)
		OnCriticalError(); // Critical error: Failed to initialize framebuffer :-(

	// Setup period irq interval before enabling interrupts
	timer_setinterval(gTimer, 1, TIMER_INTERRUPT_INTERVAL);
	timer_clearmatch(gTimer, 1);
	
	if(interrupts_init() != 0)
		printf("Failed to initialize interrupts.\n");
	
	if((result = UsbInitialise()) != 0)
		printf("Usb initialise failed, error code: %d\n", result);
	else if((result = KeyboardInitialise()) != 0)
		printf("Keyboard initialise failed, error code: %d\n", result);
			
	// Note we don't save result for Emmc as it isn't essential (yet)
	if(EmmcInitialise() != 0)
		printf("Failed to intialize emmc.\n");

	if(result == 0)
	{
		terminal_printWelcome();
				
		terminal_printPrompt();
		
		while(1)
		{
			terminal_update();
			
			wait(10);
		}
	}	
		
	print("\n * * * System Halting * * *\n", 29); 
	
	while(1);
}
