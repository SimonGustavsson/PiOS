#include "hardware/paging.h"

void kernel_pt_set(unsigned int* pt, unsigned int pa, unsigned int va, unsigned int flags)
{
    unsigned int va_base = (va >> 20) & 0xFFF;
    unsigned int pa_base = (pa >> 20) & 0xFFF;

    *(pt + va_base) = (pa_base << 20) | PAGE_PRESENT | PAGE_AP_SVCRW | flags | PT_TYPE_SECTION;
}

int kernel_pt_initialize(unsigned int* pt)
{
    // Kernel 
    unsigned int i;
    for (i = 0; i < KRL_LEVEL1_ENTRIES; i++)
        *(pt + i) = PT_TYPE_FAULT; // STMIA?

    // Map first 200MB of kernels Virtual memory to the first 200 MB in physical memory
    // So that Virtual address 0x80000000 = Physical 0x00000000
    for (i = 0; i < 200; i++)
        kernel_pt_set(pt, KERNEL_PA_START + (100000 * i), KERNEL_VA_START + (100000 * i), PAGE_CACHEABLE | PAGE_BUFFERABLE);

    // Map peripherals 
    for (i = 0; i < 256; i++)
        kernel_pt_set(pt, 0x20000000 + (100000 * i), PERIPHERAL_VA_START + (100000 * i), 0);

    //
    // TODO: 
    //     * Install PT into TTB1
    //     * Set n in TTBCR to 2
    //

    // TODO: Install temporary kernel table in TTB0 during kernel startup?

    return 0;
}

// Note: we expect pt to point towards section of USR_PT_SIZE allocated bytes
int user_pt_initialize(unsigned int* pt, unsigned int physical_start)
{
    // The level 2 table is stored right after the level 1 entry
    unsigned int first_lvl2_addr = (unsigned int)(pt + 1);
    unsigned int first_lvl2_addr_base = (first_lvl2_addr >> 10);

    // Add one level 1 entry, user processes are currently limited to 1 MB :-)
    *pt = (first_lvl2_addr_base << 10) | PAGE_AP_RW | PAGE_PRESENT | PAGE_CACHEABLE | PAGE_BUFFERABLE | PT_TYPE_COARSE;

    // Now start setting up the level 2 entries for this page
    unsigned int pa_start_base = physical_start >> 12;

    // Create 256 small pages in the table whose address we set up in the level 1 entry
    unsigned int i;
    for (i = 0; i < USR_PT_LVL2_ENTRIES_PER_LVL1; i++)
        *(unsigned int*)(first_lvl2_addr + i) = (pa_start_base << 12) | PAGE_BUFFERABLE | PAGE_CACHEABLE | SMALLPAGE_AP_RW | PT_TYPE_SMALLPAGE;

    //
    // TODO: Install user PT into TTB0
    //

    return 0;
}
