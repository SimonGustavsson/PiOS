#include "emmc.h"
#include "gpio.h"
#include "interrupts.h"
//#include "keyboard.h"
#include "stringutil.h"
#include "terminal.h"
#include "timer.h"
//#include "usbd/usbd.h"
#include "mmu.h"
#include "uart.h"
#include "fat32.h"
#include "memory.h"
#include "utilities.h"
#include "taskScheduler.h"

// Windows doesn't have __attribute__ :(
#ifdef _MSC_VER
#define __attribute__(a)
#endif

#define FINAL_USER_START 0x0A827000

// user_code gets relocated to a section in memory where user mode has access
#define user_code __attribute__((section(".user")))

extern void enable_irq(void);

//volatile unsigned int irq_counter;
volatile extern Emmc* gEmmc;
//volatile unsigned int gUserConnected;
volatile unsigned int gSystemInitialized;
volatile unsigned int gTaskSchedulerTick;

volatile unsigned int *stackAddr;

volatile extern unsigned int _user_start;
volatile extern unsigned int _user_end;

volatile extern unsigned int _bss_start;
volatile extern unsigned int _bss_end;

void dummy_proc1(void) user_code;
void dummy_proc2(void) user_code;

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

void c_undefined_handler(void* addr)
{
	printf("Undefined instruction at 0x%h. (instruction: %d\n", addr, *((unsigned int*)addr));

    wait(200);
}

void print_abort_error(unsigned int errorType)
{
	switch (errorType)
	{
	case 0x5:
		printf("Translation fault.\n");
		break;
	case 0x8:
		printf("External abort on noncachable.\n");
		break;
	case 0x9:
		printf("Failed to access memory, domain error.\n");
		break;
	case 0xD:
		printf("Permission denied accessing memory\n");
		break;
	default:
		if ((errorType >> 0x2) == 0)
			printf("Abort misaligned memory access.\n");
		else
			printf("Unknown abort exception error code: %d.\n", errorType);
		break;
	}
}

void c_abort_data_handler(unsigned int address, unsigned int errorType, unsigned int accessedAddr)
{
	printf("Instruction at 0x%h caused a data abort accessing memory at 0x%h, ", address, accessedAddr);

	print_abort_error(errorType);
}

void c_abort_instruction_handler(unsigned int address, unsigned int errorType)
{
	printf("Instruction at 0x%h caused instruction abort ", address);

	print_abort_error(errorType);
}

void c_swi_handler(unsigned int swi, unsigned int* stack, unsigned int* lr)
{
	switch (swi)
	{
	case 95:
		// Print example
		printf("Swi example print call(95).\n");
		break;
	default:
		printf("Unhandled SWI call: %d, lr = 0x%h\n", swi, lr);
		break;
	}
}

void c_irq_handler (volatile unsigned int* r0)
{
	unsigned int pendingIrq = arm_irq_getPending();

	switch (pendingIrq)
	{
		case interrupt_source_system_timer:
        {
            stackAddr = r0;

            // Note IRQ has no acccess to peripherals :(

            taskScheduler_TimerTick((registers*)r0);
			
            // Restart the timer again
            timer_sp_clearmatch();
            timer_sp_setinterval(TASK_SCHEDULER_TICK_MS);

			gTaskSchedulerTick = 1;
			break;
		}
		case interrupt_source_uart:
		{
			unsigned char read = uart_getc();
			uart_putc(read);

			if (read == 'x')
			{
				uart_puts("\r\n* * * Rebooting. * * *\r\n");
				reboot();
			}
			break;
		}
		default:
			printf("Unhandled IRQ pending, id:%d.\n", pendingIrq);
			break;
	}
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
	
	taskScheduler_Init();

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
	//if(EmmcInitialise() != 0)
	//	printf("Failed to intialise emmc.\n");
	printf("NOTE: NOT Initializing emmc and FAT32.\n");

	//if(fat32_initialize() != 0)
		//printf("Failed to initialize fat32.\n");
	
	printf("System initialization complete, result: %d\n", result);

	gSystemInitialized = 1;

	return result;
}

void dummy_proc1(void)
{
    asm volatile("SVC 95");
}

void dummy_proc2(void)
{
	for (;;)
	{
		wait(100);
		asm volatile("SVC 97");
	}
}

void move_user_code(void)
{
    unsigned int* userStart = (unsigned int*)&_user_start;
    unsigned int userLength = (&_user_end) - (&_user_start);

    printf("my_memcpy(0x%h, 0x%h, %d)", (unsigned int*)FINAL_USER_START, userStart, userLength);
    
    my_memcpy((unsigned int*)FINAL_USER_START, userStart, userLength);
}

int cmain(void)
{
    stackAddr = 0;

	system_initialize_serial();

	uart_puts("Welcome to PiOS!\n\r");

	if(system_initialize() == 0)
	{
		// Kick off the terminal
		terminal_printWelcome();
		terminal_printPrompt();

		// Test memory protection by accessing non-mapped memory
		//printf("Testing translation fault by accessing unmapped memory.\n");
		
		*((unsigned int*)0x10E00000) = 2;

		printf("If you can see this, the data abort was successful.\n");

        // Move dummy procs into user accessible memory
        move_user_code();

        void(*dummy_function1)(void) = (void(*)(void))FINAL_USER_START;
        
        // This currently doesn't work, it never returns
        dummy_function1();
        
        printf("Done with user code! :O\n");

		// Enable timer intterrupts and set up timer
        timer_sp_clearmatch();
        timer_sp_setinterval(TASK_SCHEDULER_TICK_MS);
		arm_irq_enable(interrupt_source_system_timer);

        while (1)
        {
            terminal_update();

            wait(200);
        }
	}	

	print("\n * * * System Halting * * *\n", 29); 

	while(1);
}
