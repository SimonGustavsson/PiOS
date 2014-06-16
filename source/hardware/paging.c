#include "hardware/paging.h"
#include "asm.h"
#include "types/string.h"
#include "hardware/uart.h"

// NOTE: This is placed in low memory and NOT acessible after initialization
void kernel_pt_initialize_set(unsigned int* pt, unsigned int pa, unsigned int va, unsigned int flags)
{
    unsigned int va_base = (va >> 20) & 0xFFF;
    unsigned int pa_base = (pa >> 20) & 0xFFF;

    *(pt + va_base) = (pa_base << 20) | PAGE_PRESENT | PAGE_AP_SVCRW | flags | PT_TYPE_SECTION;
}

// Use this to set the PT
void kernel_pt_set(unsigned int* pt, unsigned int pa, unsigned int va, unsigned int flags)
{
    unsigned int va_base = (va >> 20) & 0xFFF;
    unsigned int pa_base = (pa >> 20) & 0xFFF;

    *(pt + va_base) = (pa_base << 20) | PAGE_PRESENT | PAGE_AP_SVCRW | flags | PT_TYPE_SECTION;
}

int kernel_pt_initialize(unsigned int* ttb1, unsigned int* tmp_ttb0)
{
    // Kernel 
    unsigned int i;
    for (i = 0; i < KRL_LEVEL1_ENTRIES; i++)
        *(ttb1 + i) = PT_TYPE_FAULT; // STMIA?

    // First things first - Create the persistent TTB1 and fill it with 1MB sections covering the first 200MB
    for (i = 0; i < 200; i++)
        kernel_pt_initialize_set(ttb1, (i << 20), KERNEL_VA_START + (i << 20), PAGE_CACHEABLE | PAGE_BUFFERABLE);
    
    // Addtionally, add 256 1MB sections to cover the peripherals
    for (i = 0; i < 256; i++)
        kernel_pt_initialize_set(ttb1, PERIPHERAL_PA_START + (i << 20), PERIPHERAL_VA_START + (i << 20), 0);

    // Create temporary ttb0 with identity mapping that will be used
    // during the very early stages of boot while we're enabling paging
    // and before we have a chance to jump into the high-memory mapping of the kernel
    // Note that TTB0 does NOT map the peripherals, we have to jump to high-memory before accessing them
    for (i = 0; i < 20; i++)
        kernel_pt_initialize_set(tmp_ttb0, (i << 20), (i << 20), PAGE_CACHEABLE | PAGE_BUFFERABLE);
    
    // Qemu Frame buffer, starts at 0x30200000, not sure how big it is?
    //for(i = 0; i < 100; i++)
    //    kernel_pt_set(tmp_ttb0, 0x30200000 + (i << 20), 0x30200000 + (i << 20), 0);
    

    return 0;
}

// Note: we expect pt to point towards section of USR_PT_SIZE allocated bytes
//       This does NOT set TTB0 - call ttb0_set() to activate it
int user_pt_initialize(unsigned int* pt, unsigned int physical_start)
{
    // Initialize the page table to page fault
    unsigned int i;
    for (i = 0; i < 2048; i++)
        *pt = 0;

    // I can't seem to figure out why I'm getting a translation fault using the Coarse page table
    // Code below, and for now it doesn't really matter which type of table I use, so for now I decided
    // to just use sections for the time being and return to use Coarse page tables when there's a need for it
    //*pt = (((physical_start >> 20) & 0xFFF) << 20) | PAGE_PRESENT | PAGE_AP_SVCRW | PAGE_BUFFERABLE | PAGE_CACHEABLE | PT_TYPE_SECTION;

    //return 0;
    
    // The level 2 table is stored right after the level 1 table (which is 2048 entries large - Covering 2 GB)
    unsigned int first_lvl2_addr = (unsigned int)(pt + 2048);
    unsigned int first_lvl2_addr_base = first_lvl2_addr & 0xFFFFFC00; // Mask off bottom 10 bits (as it's used by flags)

    // Add one level 1 entry, user processes are currently limited to 1 MB :-)
    *pt = first_lvl2_addr_base | PAGE_PRESENT | PT_TYPE_COARSE;

    // Now start setting up the level 2 entries for this page
    unsigned int pa_start_base = physical_start >> 12;

    // Create 256 small pages in the table whose address we set up in the level 1 entry
    for (i = 0; i < USR_PT_LVL2_ENTRIES_PER_LVL1; i++)
        *(unsigned int*)(first_lvl2_addr + i) = (pa_start_base << 12) | PAGE_BUFFERABLE | PAGE_CACHEABLE | SMALLPAGE_AP_RW | PT_TYPE_SMALLPAGE;

    return 0;
}
