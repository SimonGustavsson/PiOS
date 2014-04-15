#include "hardware/emmc.h"
#include "hardware/interrupts.h"
#include "hardware/timer.h"
#include "hardware/mmu.h"
#include "hardware/uart.h"
#include "types/string.h"
#include "util/utilities.h"
#include "terminal.h"
#include "taskScheduler.h"
#include "memory.h"
#include "hardware/device/sdBlockDevice.h"
#include "fs/fs.h"
#include "fs/fat32driver.h"
#include "util/utilities.h"

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

// This variable makes sure we have something in the .data section,
// Because we place .bss before data, this will make sure that the compiler
// Zeroes out the .bss region for us, as it pads it with 0's
volatile unsigned int dataVarForPadding = 42;

extern void enable_irq(void);
extern void branchTo(unsigned int*);

volatile extern Emmc* gEmmc;
BlockDevice* gSd;

void system_initialize_serial(void)
{
	Uart_Initialize();

	Uart_EnableInterrupts();

	Arm_InterruptInitialize();

	Arm_IrqDisableall();
	Arm_IrqEnable(interrupt_source_uart);

	enable_irq();
}

int system_initialize(void)
{
	int result = 0;

    system_initialize_serial();

    Uart_SendString("Welcome to PiOS!\n\r");
    	
    // Initialize terminal first so we can print error messages if any (Hah, unlikely!)
	if ((result = Terminal_Initialize()) != 0)
	{
		Uart_SendString("Failed to initialize terminal.\n");
	}
    
    // Now that the terminal is setup, enable virtual memory
    unsigned int* basePageTable = (unsigned int *)0x000F8000;
    Mmu_Initialize(basePageTable);

    Pallocator_Initialize();

    // Verify page table by attempting to access unmapped memory
    printf("Testing translation fault by accessing unmapped memory...\n");
    *((unsigned int*)0x10E00000) = 2;
	
    // Initialize the SD card and filesystem
    gSd = (BlockDevice*)palloc(sizeof(BlockDevice));

    // Initialize the SD block device
    Sd_Register(gSd);

    // Initialize global filesystem
    fs_initialize();

    // Add support for FAT32 partitions to filesystem
    fs_register_driver_factory(&fat32_driver_factory);

    // Add the SD card to the file system
    fs_add_device(gSd);

	//taskScheduler_Init();
	
	printf("System initialization complete, result: %d\n", result);

    return result;
}

int cmain(void)
{
    if (system_initialize() != 0)
    {
        printf("\n * * * System Halting * * *\n");

        while (1);
    }

	Terminal_PrintWelcome();
	Terminal_PrintPrompt();

    // Example of opening a file
    int handle = fs_open("/dev/sd0/dummy1.img", file_read);
    if (handle != INVALID_HANDLE)
    {
        // Find the file size
        fs_seek(handle, 0, seek_end);
        unsigned int fileSize = fs_tell(handle) & 0xFFFFFFFF;
        fs_seek(handle, 0, seek_begin);

        printf("Opened /dev/sd0/dummy1.img size: %d Reading content...\n", fileSize);
        
        char* buffer = (char*)palloc(fileSize + 1);

        // Make it print friendly
        buffer[fileSize] = 0;

        // Read the entire file
        fs_read(handle, buffer, fileSize);

        // Don't need the file anymore
        fs_close(handle);

        printf("/dev/sd0/dummy1.img read, jumping to it! :D\n");

        printf("User is at 0x%h\n", FINAL_USER_START_VA);

        // Throw it at a random location and jump to i, for fun
        my_memcpy((unsigned int*)FINAL_USER_START_VA, buffer, fileSize);

        branchTo((unsigned int *)(FINAL_USER_START_VA));

        printf("\nBack in kernel!\n");
    }

    printf("Not sure what to do now...\n");

    // Timer temporarily disabled as it messes with execution of relocated code
	// Enable timer intterrupts and set up timer
    /*timer_sp_clearmatch();
    timer_sp_setinterval(TASK_SCHEDULER_TICK_MS);
	arm_irq_enable(interrupt_source_system_timer); */
    while (1)
    {
        Terminal_Update();

        wait(200);
    }
}
