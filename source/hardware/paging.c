#include "hardware/paging.h"
#include "asm.h"
#include "types/string.h"
#include "hardware/mmu_c.h"

void kernel_pt_set(unsigned int* pt, unsigned int pa, unsigned int va, unsigned int flags)
{
    unsigned int va_base = (va >> 20) & 0xFFF;
    unsigned int pa_base = (pa >> 20) & 0xFFF;

    *(pt + va_base) = (pa_base << 20) | PAGE_PRESENT | PAGE_AP_SVCRW | flags | PT_TYPE_SECTION;
}

int kernel_pt_initialize(unsigned int* ttb1, unsigned int* tmp_ttb0)
{
    if ((unsigned int)tmp_ttb0 >= 0xFFFFF)
    {
        printf("Invalid tmp_ttb0 address, must be < 0xFFFFF\n");
        return -1;
    }
    
    // Kernel 
    unsigned int i;
    for (i = 0; i < KRL_LEVEL1_ENTRIES; i++)
        *(ttb1 + i) = PT_TYPE_FAULT; // STMIA?

    // First things first - Create the persistent TTB1 and fill it with 1MB sections covering the first 200MB
    for (i = 0; i < 200; i++)
        kernel_pt_set(ttb1, (i << 20), (i << 20), PAGE_CACHEABLE | PAGE_BUFFERABLE);
    
    // Addtionally, add 256 1MB sections to cover the peripherals
    for (i = 0; i < 256; i++)
        kernel_pt_set(ttb1, 0x20000000 + (i << 20), 0x20000000 + (i << 20), 0);

    // Create temporary ttb0 with identity mapping that will be used
    // during the very early stages of boot while we're enabling paging
    // and before we have a chance to jump into the high-memory mapping of the kernel
    // Note that TTB0 does NOT map the peripherals, we have to jump to high-memory before accessing them
    for (i = 0; i < 200; i++)
        kernel_pt_set(tmp_ttb0, (i << 20), (i << 20), PAGE_CACHEABLE | PAGE_BUFFERABLE);

    // The comment above is a lie, but only for now... :-)
    for (i = 0; i < 256; i++)
        kernel_pt_set(tmp_ttb0, 0x20000000 + (i << 20), 0x20000000 + (i << 20), 0);

    printf("Enabling MMU, here goes nothing...\n");

    do_mmu(ttb1, tmp_ttb0, 0);
        
    return 0;
}

// Note: we expect pt to point towards section of USR_PT_SIZE allocated bytes
//       This does NOT set TTB0 - call ttb0_set() to activate it (probably have to flush the TLB when doing this)
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

    return 0;
}
