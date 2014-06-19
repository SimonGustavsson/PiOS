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
    Uart_SendString("Initialization complete. Go main!\n");

    TaskScheduler_Initialize();
    
    Terminal_PrintWelcome();
    Terminal_PrintPrompt();

    // Create two dummy tasks and add them to the scheduler
    Task* dummy1 = Task_Create("/dev/sd0/dummy1.elf", "Dummy1");
    if (dummy1 != NULL)
    {
        TaskScheduler_Enqueue(dummy1);

        // Just fire it off straight away or debugging as the scheduler isn't started now
        TaskScheduler_StartTask(dummy1);
    }

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
