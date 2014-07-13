#include "memory.h"
#include "stddef.h"
#include "scheduler.h"
#include "terminal.h"
#include "types/string.h"
#include "util/utilities.h"
#include "hardware/paging.h"
#include "asm.h"
#include "hardware/mailbox.h"

// This variable makes sure we have something in the .data section,
// Because we place .bss before data, this will make sure that the compiler
// Zeroes out the .bss region for us, as it pads it with 0's
volatile unsigned int dataVarForPadding = 42;

void foo3()
{
    unsigned int i;
    while (1)
    {
        Uart_SendString("Leep!\n");

        for (i = 0; i < 30000000; i++);
    }
}

void foo2()
{
    unsigned int i;
    while (1)
    {
        Uart_SendString("BEEP!\n");

        for (i = 0; i < 30000000; i++);
    }
}

void foo()
{
    unsigned int i;
    while (1)
    {
        Uart_SendString("Meeeeeep!\n");
     
        for (i = 0; i < 30000000; i++);
    }
}

int cmain(void)
{
    Uart_SendString("Initialization complete. Go main!\n");

    Scheduler_Initialize();
    
    Terminal_PrintWelcome();
    Terminal_PrintPrompt();

    printf("Address of foo: 0x%h\n", &foo);
    Process* fooProcess = Process_Create((unsigned int)&foo, "Foo(Test)");
    if (fooProcess == NULL)
    {
        printf("Failed to create foo task!\n");
    }
    else
    {
        Scheduler_Enqueue(fooProcess);
    }

    printf("Address of foo2: 0x%h\n", &foo2);
    Process* fooProcess2 = Process_Create((unsigned int)&foo2, "Foo2(Test)");
    if (fooProcess2 == NULL)
    {
        printf("Failed to create foo2 task!\n");
    }
    else
    {
        Scheduler_Enqueue(fooProcess2);
    }

    printf("Address of foo3: 0x%h\n", &foo3);
    Process* fooProcess3 = Process_Create((unsigned int)&foo3, "Foo3(Test)");
    if (fooProcess3 == NULL)
    {
        printf("Failed to create foo3 task!\n");
    }
    else
    {
        Scheduler_Enqueue(fooProcess3);
    }

    printf("Starting scheduler...\n");
    Scheduler_Start();
    printf("\nNot sure what to do now...\n");

    unsigned int i;
    while (1)
    {
        Terminal_Update();

        // Wait a bit
        for (i = 0; i < 10000; i++);
    }
}
