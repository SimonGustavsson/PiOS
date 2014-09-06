#include "asm.h"
#include "atags.h"
#include "debugging.h"
#include "fs/fat32driver.h"
#include "fs/fs.h"
#include "init.h"
#include "framebuffer.h"
#include "interrupts.h"
#include "uart.h"
#include "paging.h"
#include "timer.h"
#include "hardware/device/sdBlockDevice.h"
#include "main.h"
#include "mem.h"
#include "memory.h"
#include "memory_map.h"
#include "syscalls.h"
#include "types/string.h"
#include "terminal.h"

extern unsigned int LNK_KERNEL_END;

__attribute__((naked, aligned(32))) static void interrupt_vector(void)
{
        asm volatile("b reset\n" // Reset
        "b undefined\n"          // Undefined
        "b swi\n"                // SWI
        "b instruction_abort\n" // Instruction abort
        "b data_abort \n"        // Data abort
        "b hang\n"             // Unused
        "b irq\n"               // IRQ
        "b hang\n"                // FIQ
    );
}

void sysinit_stage1(int machineType, int aTagsPA, int dbgsymboladdr)
{
    // First thing we want to do is map the kernel and peripherals into high memory
    unsigned int* basePageTable = (unsigned int *)KERNEL_PA_PT;
    unsigned int* tmp_ttb0 = (unsigned int*)KERNEL_PA_TMP_TTB0;

    // (This also sets up a temporary mapping for ttb0 that we trash once we're in high memory)
    INIT_kernel_tt_setup(basePageTable, tmp_ttb0);

    // Enable MMU - Note this MUST be called from the sysinit() function chain
    // As this function never returns. If called from a returning function
    // That messes with the Frame Pointer (basically any C function)
    // The return from that function will reset SP to the physical address of the
    // FP and not the Virtual address set up by do_mmu
    do_mmu(tmp_ttb0, basePageTable, TTBC_SPLIT_8KB);

    // Memory is all set up, time to branch into high memory
    // to move to stage 2 of initialization
    sysinit_stage2(machineType, aTagsPA, dbgsymboladdr);
}

void sysinit_stage2(int machineType, int atagsPa, int dbgsymboladdr)
{
    // Setup the interrupt vector
    asm volatile("mcr p15, 0, %[addr], c12, c0, 0" : : [addr] "r" (&interrupt_vector));

    // TODO: Trash the temporary TTB0, we shouldn't need it past this point
    //       Currently I think the emmc driver uses a hardcoded buffer in low memory
    unsigned int cur = 0;
    for (cur = 0; cur < 8192; cur++)
        *(char*)KERNEL_PA_TMP_TTB0 = 0;
    
    // First things first, enable the serial so we can send the deployer some feedback
    Uart_Initialize();
    Uart_EnableInterrupts();
    Arm_InterruptInitialize();
    Arm_IrqDisableall();
    Arm_IrqEnable(interrupt_source_uart);
    enable_irq();

    Uart_SendString("Welcome to PiOS!\n\n");

    Pallocator_Initialize();

    // Initialize page allocator
    mem_init();

    // Calculate kernel size
    unsigned int kernel_physical_end = (unsigned int)&LNK_KERNEL_END;
    kernel_physical_end -= KERNEL_VA_START;
    unsigned int kernel_size = kernel_physical_end - LD_KRNL_ORIGIN;

    // Reserve kernel regions of the memory in the page allocator
    mem_reserve(0x0, 0x2000); // Reserve the first 3 pages, contains boot parameters and such
    mem_reserve(FIQ_STACK_PA_START - SMALL_STACK_SIZE, SMALL_STACK_SIZE);
    mem_reserve(0x5000, 0x100);
    mem_reserve(IRQ_STACK_PA_START - SMALL_STACK_SIZE, SMALL_STACK_SIZE);
    mem_reserve(LD_KRNL_ORIGIN, kernel_size);
    mem_reserve(KERNEL_PA_PT, TTB_SIZE_4GB_SIZE + (TTB_SIZE_4GB_SIZE * 256));
    mem_reserve(SVC_STACK_PA_START - SMALL_STACK_SIZE, SMALL_STACK_SIZE);
    mem_reserve(UD_STACK_PA_START - SMALL_STACK_SIZE, SMALL_STACK_SIZE);
    mem_reserve(ABORT_STACK_PA_START - SMALL_STACK_SIZE, SMALL_STACK_SIZE);
    // Pallocator (TODO: Pallocator should dynamically request pages as and when needed
    //                    instead of reserving a mahossive chunk straight up)
    mem_reserve(DYN_MEM_PA_START, MAX_ALLOCATED_BYTES);

    mem_reserve(SM_STACK_PA_START - SMALL_STACK_SIZE, SMALL_STACK_SIZE);
    // TODO: Need to allocate all static things, see memory_map.h

    char* dbgSymbolsVa = (char*)(KERNEL_VA_START + dbgsymboladdr);
    Debug_ReadFunctionNames(dbgSymbolsVa);
    // Initialize terminal first so we can print error messages if any (Hah, unlikely!)
    if(Terminal_Initialize() != 0)
    {
        Uart_SendString("Failed to initialize terminal\n");
    }

    // Reserve physical pages for framebuffer
    mem_reserve(Fb_GetPhyAddr(), Fb_GetSize());

    Fb_Clear();
    Terminal_Clear();
    
    printf("Kernel size: %d\n", kernel_size);

    // Example usage of timer to measure performance
    long long start = Timer_GetTicks();

    // Initialize the SD card and filesystem
    BlockDevice* sd = (BlockDevice*)palloc(sizeof(BlockDevice));

    if(sd == 0)
    {
        Uart_SendString("Failed to allocate memory for SD device\n");
    }
    else
    {
        // Initialize the SD block device
        //Sd_Register(sd);

        // Initialize global filesystem
        //fs_initialize();

        // Add support for FAT32 partitions to filesystem
        //fs_register_driver_factory(&fat32_driver_factory);

        // Add the SD card to the file system
        //fs_add_device(sd);
    }

    long long end = Timer_GetTicks();

    // (This should be long long, but printf doesn't support that yet, also no long long div func)
    unsigned int time_taken = ((unsigned int)(end - start)) / 1000;
    printf("It took %dms to initialize the SD card and file system.\n", time_taken);

    // Show some usage of reserved memory at boot now that we're done reserving
    mem_printUsage();
    
    printf("Our machine type is: 0x%h\n", machineType);
    printf("ATAGS start at 0x%h\n", atagsPa);
    printf("Debugging symbols are at 0x%h\n", dbgsymboladdr);

    atags_parse((int*)(KERNEL_VA_START + atagsPa));

    syscalls_init();

    // Enter... the kernel!
    cmain();
}