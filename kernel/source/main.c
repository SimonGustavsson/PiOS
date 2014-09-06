#include "memory.h"
#include "stddef.h"
#include "scheduler.h"
#include "terminal.h"
#include "types/string.h"
#include "util/utilities.h"
#include "util/memutil.h"
#include "paging.h"
#include "asm.h"
#include "mailbox.h"
#include "syscalls.h"
#include "memory_map.h"
#include "framebuffer.h"

// This variable makes sure we have something in the .data section,
// Because we place .bss before data, this will make sure that the compiler
// Zeroes out the .bss region for us, as it pads it with 0's
volatile unsigned int dataVarForPadding = 42;

void foo3()
{
    unsigned int i;
    unsigned int num = 0;
    while (1)
    {
        printf("\rFoo3(%d)", num);
        num++;

        for (i = 0; i < 30000000; i++);

        // volatile unsigned int asd = *(int*)(0x1000 + 4092);
        // volatile unsigned int sff = *(int*)(0x2000 + 4092);
        // volatile unsigned int fasa = *(int*)(0x3000 + 4092);
      //  printf("Values: 0x%h, 0x%h, 0x%h\n", asd, sff, fasa);
    }
}

void foo2()
{
    unsigned int i;
    unsigned int num = 0;
    while (1)
    {
        printf("\rFoo2(%d)", num);
        num++;

        for (i = 0; i < 30000000; i++);

        // volatile unsigned int asd = *(int*)(0x1000 + 4092);
        // volatile unsigned int sff = *(int*)(0x2000 + 4092);
        // volatile unsigned int fasa = *(int*)(0x3000 + 4092);
    //    printf("Values: 0x%h, 0x%h, 0x%h\n", asd, sff, fasa);
    }
}

void foo()
{
    //thread* child = pios_thread_create((thread_entry)&foo3);

    //unsigned int va = KERNEL_VA_START + Scheduler_GetCurrentThread()->owner->mem_pages[0];
    //printf("0x%h = 0x%h\n", va, *(unsigned int*)va);

    unsigned int i;
    unsigned int num = 0;
    while (1)
    {
        printf("\rFoo(%d)", num);
        num++;
     
        for (i = 0; i < 30000000; i++);

        // volatile unsigned int asd =  *(int*)(0x1000 + 4092);
        // volatile unsigned int sff =  *(int*)(0x2000 + 4092);
        // volatile unsigned int fasa = *(int*)(0x3000 + 4092);    
        // printf("Values: 0x%h, 0x%h, 0x%h\n", asd, sff, fasa);
    }
}

void test_ttb0(void)
{
    Process* p = Process_Create((unsigned int)&foo, "TTB0 Test", false);

    printf("Setting ttbc to 0x%h\n", p->ttb0_size);
    printf("Switching in ttb0, address: 0x%h\n", p->ttb0_physical);
    unsigned int ttbc = get_ttbc();
    ttbc &= p->ttb0_size;
    set_ttbc(ttbc);
    set_ttb0(p->ttb0_physical, 1);

    FlushCache();
    InvalidateAllUnlockedTLB();
    FlushTLB(0x1000);

    printf("First entries: 0x%h 0x%h 0x%h\n", p->ttb0[0], p->ttb0[1], p->ttb0[2]);

    unsigned int bytesToCheck = PROCESS_START_PAGE_COUNT * LD_PAGE_SIZE;

    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("Testing %d bytes (0x%h)\n", bytesToCheck, bytesToCheck);
    int k;
    for (int i = 0; i < bytesToCheck; i++)
    {
        k = *(int*)(0x1000 + i);
    }

    printf("Tested %d bytes (%d pages). No output = Good output\n", bytesToCheck, bytesToCheck / 4096);

    printf("Reading bottom of stack...\n");
    int stackBottom = *(int*)(THREAD_STACK_VA_START - 4);

    printf("Last read k: %d stack bottom: 0x%h\n", k, stackBottom);
}

int cmain(void)
{
    Uart_SendString("Initialization complete. Go main!\n");

    Scheduler_Initialize();
    
    Terminal_PrintWelcome();
    Terminal_PrintPrompt();

    Process* fooProcess = Process_Create((unsigned int)&foo, "Foo(Test)", true);
    if (fooProcess == NULL)
        printf("Failed to create foo task!\n");

    Process* fooProcess2 = Process_Create((unsigned int)&foo2, "Foo2(Test)", true);
    if (fooProcess2 == NULL)
        printf("Failed to create foo2 task!\n" );
    

    Process* fooProcess3 = Process_Create((unsigned int)&foo3, "Foo3(Test)", true);
    if (fooProcess3 == NULL)
       printf("Failed to create foo3 task!\n");

    printf("Starting scheduler...\n");
    Scheduler_Start();

    unsigned int i;
    while (1)
    {
        Terminal_Update();

        // Wait a bit
        for (i = 0; i < 10000; i++);
    }
}
