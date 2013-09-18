#include "emmc.h"
#include "gpio.h"
#include "interrupts.h"
#include "keyboard.h"
#include "stringutil.h"
#include "terminal.h"
#include "timer.h"
#include "usbd/usbd.h"

volatile unsigned int irq_counter;

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
	
	// Reset the system periodic timer
	timer_sp_clearmatch();
	timer_sp_setinterval(TIMER_INTERRUPT_INTERVAL);
}

unsigned int system_initialize(void)
{
	unsigned int result = 0;
	
	// First and foremost: The LED so we can flash it to signal errors if FB init fails
	LedInit();
	
	// Initialize terminal first so we can print error messages if any (Hah, unlikely!)
	if((result = terminal_init()) != 0)
		OnCriticalError(); // Critical error: Failed to initialize framebuffer :-(

	// Note: Timer is not essential to system initialisation
	if(timer_init() != 0)
		printf("Failed to initialise timer.\n");
	
	// Note: Interrupts are not essential to system initialisation
	if(interrupts_init() != 0)
		printf("Failed to initialize interrupts.\n");
	
	// Note: Usb & Keyboard is essential to the system
	if((result = UsbInitialise()) != 0)
		printf("Usb initialise failed, error code: %d\n", result);
	else if((result = KeyboardInitialise()) != 0)
		printf("Keyboard initialise failed, error code: %d\n", result);

	// Note: EMMC is not essential to system initialisation
	if(EmmcInitialise() != 0)
		printf("Failed to intialize emmc.\n");
	
	return result;
}

int cmain(void)
{
	irq_counter = 0;
		
	if(system_initialize() == 0)
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
