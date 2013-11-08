#include "emmc.h"
#include "gpio.h"
#include "interrupts.h"
#include "keyboard.h"
#include "stringutil.h"
#include "terminal.h"
#include "timer.h"
#include "usbd/usbd.h"
#include "uart.h"
#include "fat32.h"

extern void enable_irq(void);

//volatile unsigned int irq_counter;
volatile extern Emmc* gEmmc;
volatile unsigned int gUserConnected;

void OnCriticalError(void)
{
	while(1)
	{
		//LedOff();

		//wait(1000);		

		//LedOn();		

		//wait(1000);
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
	unsigned char read = uart_getc();

	gUserConnected = 1;

	// If this was triggered by the timer
	// Reset the system periodic timer
	//timer_sp_clearmatch();
	//timer_sp_setinterval(TIMER_INTERRUPT_INTERVAL);
}

void system_initialize_serial(void)
{
	uart_init();

	uart_irpt_enable();

	arm_interrupt_init();

	arm_irq_disableall();
	arm_irq_enable(interrupt_source_uart);

	enable_irq();
}

unsigned int system_initialize(void)
{
	unsigned int result = 0;
	
	// Initialize terminal first so we can print error messages if any (Hah, unlikely!)
	if ((result = terminal_init()) != 0)
	{
		uart_puts("Failed to initialize terminal.\n");
		OnCriticalError(); // Critical error: Failed to initialize framebuffer :-(
	}
	// Note: Timer is not essential to system initialisation
	if (timer_init() != 0)
	{
		uart_puts("Failed to initialize timer.\n");
		printf("Failed to initialise timer.\n");
	}
	
	// Note: Usb & Keyboard is essential to the system
	if((result = UsbInitialise()) != 0)
		printf("Usb initialise failed, error code: %d\n", result);
	else if((result = KeyboardInitialise()) != 0)
		printf("Keyboard initialise failed, error code: %d\n", result);

	// Note: EMMC is not essential to system initialisation
	if(EmmcInitialise() != 0)
		printf("Failed to intialise emmc.\n");

	if(fat32_initialize() != 0)
		printf("Failed to initialize fat32.\n");
	
	printf("System initialization complete, result: %d\n", result);

	return result;
}

void WaitForUartAlive(void)
{
	volatile unsigned int i;
	while (!gUserConnected)
	{
		for (i = 0; i < 150000; i++) { /* Do Nothing */ }
	}

	uart_puts("Welcome to PiOS!\n\r");
}

int cmain(void)
{
	gUserConnected = 0;

	system_initialize_serial();

	// System all up and running, wait for a alive sign from the uart before proceeding
	// TODO: Conditionalize this - only use if no screen (keyboard?) is attached
	WaitForUartAlive();

	if(system_initialize() == 0)
	{
		// Kick off the terminal
		terminal_printWelcome();
		terminal_printPrompt();
		
		while(1)
		{
			terminal_update();

			wait(20);
		}
	}	

	print("\n * * * System Halting * * *\n", 29); 

	while(1);
}
