#include "emmc.h"
#include "gpio.h"
#include "interrupts.h"
#include "keyboard.h"
#include "stringutil.h"
#include "terminal.h"
#include "timer.h"
#include "usbd/usbd.h"
#include "mini_uart.h"

volatile unsigned int irq_counter;
volatile extern Emmc* gEmmc;

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
	printf_i("IRQ no. %d\n", irq_counter);


	// Reset the system periodic timer
	//timer_sp_clearmatch();
	//timer_sp_setinterval(TIMER_INTERRUPT_INTERVAL);
}

unsigned int system_initialize(void)
{
	unsigned int result = 0;

	// First and foremost: The LED so we can flash it to signal errors if FB init fails
	gpio_initialize();
	
	// Initialize terminal first so we can print error messages if any (Hah, unlikely!)
	if((result = terminal_init()) != 0)
		OnCriticalError(); // Critical error: Failed to initialize framebuffer :-(
	
	//if(mini_uart_initialize() != 0)
	//	printf("Failed to initialize uart.\n");

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
		printf("Failed to intialise emmc.\n");

	return result;
}

void WaitForUartAlive(void)
{
	printf("Waiting for user to connect via uart...\n");

	// Block until a char is read
	mini_uart_read_char(1);

	printf("User connected, launching system.\n");

	mini_uart_send_string("Welcome, user!\n");
}

int cmain(void)
{
	irq_counter = 0;

	if(system_initialize() == 0)
	{
		// System all up and running, wait for a alive sign from the uart before proceeding
		// TODO: Conditionalize this - only use if no screen attached
		//WaitForUartAlive();

		// Kick off the terminal
		terminal_printWelcome();
		terminal_printPrompt();
		char buf[512];

		if(EmmcReadBlock(&buf[0], 512, 0))
		{
			if(buf[0x1FE] != 0x55 || buf[0x1FF] != 0xAA)
			{
				printf("Not a valid MBR, buf[0x1FE] = %d, buf[0x1FF] = %d\n", buf[0x1FE], buf[0x1FF]);
			}
			else
			{
				printf("Valid MBR detected on SD Card, woho!\n");
			}
		}

		while(1)
		{
			terminal_update();

			wait(5);
		}
	}	

	print("\n * * * System Halting * * *\n", 29); 

	while(1);
}
