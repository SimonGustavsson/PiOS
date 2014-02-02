#include "emmc.h"
#include "gpio.h"
#include "interrupts.h"
#include "keyboard.h"
#include "stringutil.h"
#include "terminal.h"
#include "timer.h"
//#include "usbd/usbd.h"
#include "mmu.h"
#include "uart.h"
#include "fat32.h"

extern void enable_irq(void);

//volatile unsigned int irq_counter;
volatile extern Emmc* gEmmc;
//volatile unsigned int gUserConnected;
volatile unsigned int gSystemInitialized;

void reboot()
{
   static const int PM_RSTC = 0x2010001c;
   static const int PM_WDOG = 0x20100024;
   static const int PM_PASSWORD = 0x5a000000;
   static const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;
   
   *(unsigned int*)(PM_WDOG) = (PM_PASSWORD | 1); // timeout = 1/16th of a second? (whatever)
   *(unsigned int*)(PM_RSTC) = (PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);

   while(1);
}

// Log function for CSUD
void LogPrint(char* message, unsigned int length)
{
	// Disabled usb driver output for now, it's not interesting enough!
	//print(message, length);
}

void c_prefetch_handler(void)
{
	printf("Prefetch abort!\n");
}

void c_data_abort_handler(void)
{
	printf("Data abort!\n");
}

void c_swi_handler(void)
{
	printf("SWI exception.\n");
}

void c_undefined_handler(void)
{
	printf("Undefined exception\n");
}

void c_irq_handler (void)
{
	// if(uart_read_irpt_pending())
	unsigned char read = uart_getc();

	uart_putc(read);

	if (read == 'x')
	{
		uart_puts("\r\n* * * Rebooting. * * *\r\n");
		reboot();
	}

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
	}

	unsigned int* basePageTable = (unsigned int *)0x00A08000;

	printf("Base page table address: %d\n", basePageTable);

	initMmu(basePageTable);
	
	// Note: Timer is not essential to system initialisation
	if (timer_init() != 0)
	{
		uart_puts("Failed to initialize timer.\n");
		printf("Failed to initialise timer.\n");
	}
	
	// Note: Usb & Keyboard is essential to the system
	//if((result = UsbInitialise()) != 0)
	//	printf("Usb initialise failed, error code: %d\n", result);
	//else if((result = KeyboardInitialise()) != 0)
	//	printf("Keyboard initialise failed, error code: %d\n", result);

	// Note: EMMC is not essential to system initialisation
	if(EmmcInitialise() != 0)
		printf("Failed to intialise emmc.\n");

	if(fat32_initialize() != 0)
		printf("Failed to initialize fat32.\n");
	
	printf("System initialization complete, result: %d\n", result);

	gSystemInitialized = 1;

	return result;
}

int cmain(void)
{
	//gUserConnected = 0;

	system_initialize_serial();

	uart_puts("Welcome to PiOS!\n\r");

	if(system_initialize() == 0)
	{
		// Kick off the terminal
		terminal_printWelcome();
		terminal_printPrompt();

		// Test memory protection by accessing non-mapped memory
		printf("About to access forbidden memory! Uh-uh...\n");

		unsigned int* abortMe = (unsigned int*)0x10E00000;

		*abortMe = 2;

		printf("Test success!\n");

		while(1)
		{
			terminal_update();

			wait(200);
		}
	}	

	print("\n * * * System Halting * * *\n", 29); 

	while(1);
}
