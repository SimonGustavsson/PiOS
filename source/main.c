#include "memory.h"
#include "stddef.h"
#include "taskScheduler.h"
#include "terminal.h"
#include "types/string.h"
#include "util/utilities.h"
#include "hardware/paging.h"
#include "asm.h"

// This variable makes sure we have something in the .data section,
// Because we place .bss before data, this will make sure that the compiler
// Zeroes out the .bss region for us, as it pads it with 0's
volatile unsigned int dataVarForPadding = 42;

int cmain(void)
{
    TaskScheduler_Initialize();

    Uart_SendString("Initialization complete. Go main!\n");

    if (Terminal_GetIsInitialized() == 0)
    {
        Uart_SendString("Failed to initialize terminal, * * * HALTING * * *\n");
        while (1);
    }

    Terminal_PrintWelcome();
    Terminal_PrintPrompt();

    // Create two dummy tasks and add them to the scheduler
    task_entry_func dummy1_entry = Task_LoadElf("/dev/sd0/dummy1.elf", USR_PA_START);
    task_entry_func dummy2_entry = Task_LoadElf("/dev/sd0/dummy2.elf", USR_PA_START + 0x100000);

    Task* dummy1 = 0;
    Task* dummy2 = 0;

    if (dummy1_entry != NULL)
    {
        dummy1 = Task_Create(dummy1_entry, "Dummy1");
        TaskScheduler_EnqueueTask(dummy1);
    }

    if (dummy2_entry != NULL)
    {
        dummy2 = Task_Create(dummy2_entry, "Dummy2");
        TaskScheduler_EnqueueTask(dummy2);
    }
    
    printf("Verifying initialization:\n");

    printf("Current stack pointer: 0x%h\n", get_sp());


    // (Temporary) Test tb0 switching
    unsigned int curAddr;
    int* test_ttb0 = (int*)USR_PA_START + 0x4000;
    int* test2_ttb0 = (int*)USR_PA_START + 0x8000;
    user_pt_initialize(test_ttb0, 0x100000);
    user_pt_initialize(test2_ttb0, 0x200000);

    set_ttb0(test_ttb0, 1);

    // Values printed here needs to be verified to values printed prior to trashing
    // the kernels temporary ttb0
    printf("= Attempting to access 0x0  = \n");
    volatile unsigned int foo = *(unsigned int*)0x0;
    printf("Value read: %d\n", foo);

    // Now switch to the second test table and read the values
    printf("Switching to second test table...\n");

    set_ttb0(test2_ttb0, 1);
    
    printf("= Attempting to access 0x0  = \n");
    volatile unsigned int foo2 = *(unsigned int*)0x0;
    printf("Value read: %d\n", foo2);

    printf("Done testing ttb0, any faults?\n");
     
    //printf("Starting task scheduler...\n");
    //TaskScheduler_Start();
    printf("\nNot sure what to do now...\n"); 
    unsigned int i;
    while (1)
    {
        Terminal_Update();

        // Wait a bit
        for (i = 0; i < 10000; i++);
    }
}
