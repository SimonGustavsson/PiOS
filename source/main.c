#include "emmc.h"
#include "interrupts.h"
#include "stringutil.h"
#include "terminal.h"
#include "timer.h"
#include "mmu.h"
#include "uart.h"
#include "fat32.h"
#include "utilities.h"
#include "taskScheduler.h"

// Windows doesn't have __attribute__ :(
#ifdef _MSC_VER
#define __attribute__(a)
#define asm 
#endif

// user_code gets relocated to a section in memory where user mode has access
#define user_code __attribute__((section(".user"))) __attribute__ ((noinline))
#define SVC_INSTRUCTION(number) asm volatile("svc %0" : : "I" (number))
#define FINAL_USER_START_VA 0x00F00000
#define task_main_func int(*)(void)

extern void enable_irq(void);
extern void branchTo(unsigned int*);

volatile extern Emmc* gEmmc;

// Log function for CSUD
void LogPrint(char* message, unsigned int length)
{
	// Disabled usb driver output for now, it's not interesting enough!
	//print(message, length);
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
	
	//taskScheduler_Init();

	// Note: EMMC is not essential to system initialisation
	//if(EmmcInitialise() != 0)
	//	printf("Failed to intialise emmc.\n");
	printf("NOTE: NOT Initializing emmc and FAT32.\n");

	//if(fat32_initialize() != 0)
		//printf("Failed to initialize fat32.\n");
	
	//printf("System initialization complete, result: %d\n", result);
	return result;
}

int cmain(void)
{
    // Init UART first so we can send boot messages to deployer
	system_initialize_serial();

	uart_puts("Welcome to PiOS!\n\r");

	if(system_initialize() == 0)
	{
		terminal_printWelcome();
		terminal_printPrompt();

		printf("Testing translation fault by accessing unmapped memory.\n");
		*((unsigned int*)0x10E00000) = 2;
		printf("If you can see this, the data abort was successful.\n");
                
        printf("Calling loaded program img, should trigger SVC!\n");
        //branchTo((unsigned int *)(FINAL_USER_START_VA));
        printf("It worked!? :-)\n");

        // Timer temporarily disabled as it messes with execution of relocated code
		// Enable timer intterrupts and set up timer
        /*timer_sp_clearmatch();
        timer_sp_setinterval(TASK_SCHEDULER_TICK_MS);
		arm_irq_enable(interrupt_source_system_timer); */
        while (1)
        {
            terminal_update();

            wait(200);
        }
	}	

	print("\n * * * System Halting * * *\n", 29); 

	while(1);
}
