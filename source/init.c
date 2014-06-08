#include "asm.h"
#include "fs/fat32driver.h"
#include "fs/fs.h"
#include "init.h"
#include "hardware/framebuffer.h"
#include "hardware/interrupts.h"
#include "hardware/uart.h"
#include "hardware/paging.h"
#include "hardware/device/sdBlockDevice.h"
#include "main.h"
#include "memory.h"
#include "memory_map.h"
#include "types/string.h"
#include "terminal.h"

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

void sysinit_stage1(void)
{
    // First thing we want to do is map the kernel and peripherals into high memory
    unsigned int* basePageTable = (unsigned int *)KERNEL_PA_PT;
    unsigned int* tmp_ttb0 = (unsigned int*)KERNEL_PA_TMP_TTB0;

    // (This also sets up a temporary mapping for ttb0 that we trash once we're in high memory)
    kernel_pt_initialize(basePageTable, tmp_ttb0);

    // Enable MMU - Note this MUST be called from the sysinit() function chain
    // As this function never returns. If called from a returning function
    // That messes with the Frame Pointer (basically any C function)
    // The return from that function will reset SP to the physical address of the
    // FP and not the Virtual address set up by do_mmu
    do_mmu(tmp_ttb0, basePageTable, TTBC_SPLIT_8KB);

    // Memory is all set up, time to branch into high memory
    // to move to stage 2 of initialization
    sysinit_stage2();
}

void sysinit_stage2(void)
{
    // Setup the interrupt vector
    asm volatile("mcr p15, 0, %[addr], c12, c0, 0" : : [addr] "r" (&interrupt_vector));

    volatile unsigned int usrStartValBefore = *(unsigned int*)0x100000;
    volatile unsigned int usr2StartValBefore = *(unsigned int*)0x200000;

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

    // Initialize terminal first so we can print error messages if any (Hah, unlikely!)
    Terminal_Initialize();

    // Now that the terminal is initialized, add a VA mapping for it
    size fbSize = Fb_GetScreenSize();    
    unsigned int fb_phy_addr = Fb_GetPhyAddr();
    unsigned int fbSizeInMB = (((fbSize.width * fbSize.height * FB_BPP) / 1024) / 1024) + 1;

    unsigned int i, addr;
    for (i = 0; i < fbSizeInMB; i++)
    {
        // Note: At this point we still have a 1:1 mapping of the kernel, so we can use
        // The physical address of the page table
        addr = fb_phy_addr + (i << 20);
        kernel_pt_set((unsigned int*)KERNEL_PT_VA_START, addr, FRAMEBUFFER_VA_START + (i << 20), 0);
        FlushTLB(addr);        
    }

    Fb_Clear();
    Terminal_Clear();

    printf("Value at 0x100000 (which is user 0x0): %d\n", usrStartValBefore);
    printf("Value at 0x200000 (which is user2 0x0): %d\n", usr2StartValBefore);
    // Verify page table by attempting to access unmapped memory
    //printf("Testing translation fault by accessing unmapped memory...\n");
    //*((unsigned int*)0x10E00000) = 2;

    // Initialize the SD card and filesystem
    BlockDevice* sd = (BlockDevice*)palloc(sizeof(BlockDevice));

    if(sd == 0)
    {
        Uart_SendString("Failed to allocate memory for SD device\n");
    }
    else
    {
        // Initialize the SD block device
        Sd_Register(sd);

        // Initialize global filesystem
        fs_initialize();

        // Add support for FAT32 partitions to filesystem
        fs_register_driver_factory(&fat32_driver_factory);

        // Add the SD card to the file system
        fs_add_device(sd);
    }
    
    // Enter... the kernel!
    cmain();
}