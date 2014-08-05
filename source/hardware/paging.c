#include "hardware/paging.h"
#include "asm.h"
#include "types/string.h"
#include "hardware/uart.h"
#include "mem.h"

void map_page(unsigned int* pt, unsigned int num_lvl1_entries, unsigned int pa, unsigned int va, unsigned int flags)
{
    unsigned int* physical_tt = (pt - KERNEL_VA_START);

    // The shifts of the virtual address explained:
    // 31:20 map into lvl 1 table
    // 19:12 map into lvl 2 page
    // 11:0 index into the lvl 2 page (Not relevant to us)
    unsigned int lvl1_index = va >> 20;
    unsigned int* lvl1_entry = (physical_tt + lvl1_index);
    unsigned int* lvl1_entry_va = (pt + lvl1_index);
    
    // TODO: Check to make sure that it actually fits into the page table, or if we need to expand it (reallocate a series of more pages, update ttbc etc)
    // Level 2 entries start on the first clean page after the end of the level 1 entries
    unsigned int lvl1_entries_num_pages = num_lvl1_entries / PAGE_SIZE;

    // Might not fit exactly into pages, add one if the level 1 entries spills over into a page
    lvl1_entries_num_pages += (num_lvl1_entries % PAGE_SIZE) == 0 ? 0 : 1;

    unsigned int level2_entries_start_addr = ((unsigned int)physical_tt) + (lvl1_entries_num_pages * PAGE_SIZE);
    unsigned int level2_entries_start_addr_va = level2_entries_start_addr + KERNEL_VA_START;

    unsigned int lvl2_index = (va >> 12) & 0xFF;
    unsigned int* lvl2_entry = (unsigned int*)(level2_entries_start_addr + ( (lvl1_index * 256) * sizeof(int)   ) + (lvl2_index << 2      )   );
    unsigned int* lvl2_entry_va = lvl2_entry + KERNEL_VA_START;
    
    // Make sure the level 1 entry is initialized
    if ((*lvl1_entry_va & PAGE_TABLE_MASK) == 0)
    {
        *lvl1_entry_va = (((unsigned int)lvl2_entry) & 0xFFFFFC00) | PAGE_TABLE_COARSE;
        //*lvl1_entry_va = ((((unsigned int)lvl2_entry) & 0x3FFFFF) << 10) | PAGE_TABLE_COARSE;
    }

    unsigned int entry = (pa & 0xFFFFF000) | PAGE_SMALL | PAGE_EXECUTE_NEVER | flags;

    // Set the value of the level 2 entry
    *lvl2_entry_va = entry;

#ifdef PAGEMEM_DEBUG
    printf("Mapped 0x%h (0x%h) to 0x%h: lvl1: 0x%h(at 0x%h) lvl2: 0x%h(at 0x%h) ~ TT addr: 0x%h\n", pa, (pa & 0xFFFFF000), va, *lvl1_entry_va, lvl1_entry, 
        *lvl2_entry_va, lvl2_entry, pt);
#endif
}

void map_section(unsigned int* pt, unsigned int pa, unsigned int va, unsigned int flags)
{
    unsigned int va_base = (va >> 20) & 0xFFF;
    unsigned int pa_base = (pa >> 20) & 0xFFF;

    *(pt + va_base) = (pa_base << 20) | PAGE_TABLE_SECTION | flags;
}

// NOTE: This is placed in low memory and NOT acessible after initialization
void INIT_map_section(unsigned int* pt, unsigned int pa, unsigned int va, unsigned int flags)
{
    unsigned int va_base = (va >> 20) & 0xFFF;
    unsigned int pa_base = (pa >> 20) & 0xFFF;

    *(pt + va_base) = (pa_base << 20) | PAGE_TABLE_SECTION | flags;
}

int INIT_kernel_tt_setup(unsigned int* ttb1, unsigned int* tmp_ttb0)
{
    // Kernel 
    unsigned int i;
    for (i = 0; i < TTB_SIZE_4GB_SIZE; i++)
        *(ttb1 + i) = PAGE_TABLE_FAULT; // STMIA?
    
    // First things first - Create the persistent TTB1 and fill it with 1MB sections covering the first 200MB
    for (i = 0; i < 200; i++)
        INIT_map_section(ttb1, (i << 20), KERNEL_VA_START + (i << 20), PAGE_CACHEABLE | PAGE_BUFFERABLE | SECTION_AP_K_RW);
    
    // Addtionally, add 256 1MB sections to cover the peripherals
    for (i = 0; i < 256; i++)
        INIT_map_section(ttb1, PERIPHERAL_PA_START + (i << 20), PERIPHERAL_VA_START + (i << 20), SECTION_AP_K_RW);

    // Create temporary ttb0 with identity mapping that will be used
    // during the very early stages of boot while we're enabling paging
    // and before we have a chance to jump into the high-memory mapping of the kernel
    // Note that TTB0 does NOT map the peripherals, we have to jump to high-memory before accessing them
    for (i = 0; i < 20; i++)
        INIT_map_section(tmp_ttb0, (i << 20), (i << 20), PAGE_CACHEABLE | PAGE_BUFFERABLE | SECTION_AP_K_RW);
    
    // Qemu Frame buffer
    unsigned int qemuFbAddress2 = 0x1C100000;
    for(i = 0; i < 2; i++)
        INIT_map_section(ttb1, qemuFbAddress2 + (i << 20), FRAMEBUFFER_VA_START + (i << 20), SECTION_AP_K_RW);

    return 0;
}

void mem_print_page_va_info(unsigned int* pt, unsigned int va)
{
    // This function basically does the same as the MMU would when performing a
    // second level page table walk over Extended 4KB ARMv6 small pages.
    // Note: The pt param is assumed to point to a Translation table containing coarse table entries.
    printf("Info for Virtual Address: 0x%h\n", va);
    printf("Translation table base: 0x%h\n", pt);

    unsigned int first_level_table_index = (va >> 20) & 0xFFF;
    unsigned int second_level_table_index = (va >> 12) & 0x7F;
    unsigned int page_index = (va & 0xFFF);
    printf("First level table index: 0x%h\n", first_level_table_index);
    printf("Second level table index: 0x%h\n", second_level_table_index);
    printf("Page index: 0x%h\n", page_index);

    unsigned int* first_level_descriptor = (unsigned int*)(((unsigned int)pt) + (first_level_table_index << 2));
    printf("First level descriptor address: 0x%h\n", first_level_descriptor);
    printf("First level descriptor: 0x%h\n", *first_level_descriptor);

    unsigned int coarse_table_base_addr = ((*first_level_descriptor >> 10) & 0x3FFFFF);
    printf("Coarse table base address: 0x%h ( 0x%h )\n", coarse_table_base_addr, (coarse_table_base_addr << 10));

    unsigned int* second_level_descriptor = (unsigned int*)((coarse_table_base_addr << 10) | (second_level_table_index << 2));

    printf("Second level descriptor address: 0x%h\n", second_level_descriptor);
    printf("Second level descriptor: 0x%h\n", *second_level_descriptor);

    unsigned int physical_address = ((*second_level_descriptor & 0xFFFFF000) | page_index);

    // Note: this relies on the kernel ttb1 to identity map physical memory to KERNEL_VA_START
    printf("Physical address: 0x%h, Value: 0x%h\n", physical_address, *((unsigned int*)(KERNEL_VA_START + physical_address)));
}
