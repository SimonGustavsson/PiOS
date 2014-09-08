#ifndef MMU_H
#define MMU_H

#include "stdint.h"

// "Size" is the size of the address space it covers, value is number of level 1 entries
#define TTB_SIZE_4GB_SIZE   4096
#define TTB_SIZE_2GB_SIZE   2048
#define TTB_SIZE_1GB_SIZE   1024
#define TTB_SIZE_512MB_SIZE  512
#define TTB_SIZE_256MB_SIZE  256
#define TTB_SIZE_128MB_SIZE  128
#define TTB_SIZE_64MB_SIZE    64
#define TTB_SIZE_32MB_SIZE    32

// TODO: This is ARM specific? Move into arch
typedef enum {
    ttbc_16KB     = 0, // 4GB Address space
    ttbc_8KB      = 1, // 2GB Address space
    ttbc_4KB      = 2, // 1GB Address space
    ttbc_2KB      = 3, // 512MB Adress space
    ttbc_1KB      = 4, // 256MB Address sapce
    ttbc_512byte  = 5, // 128MB Address space
    ttbc_256byte  = 6, // 64MB Address space
    ttbc_128bytes = 7  // 32MB Address space
} ttbc_ttbr0_size;

// Creates a section mapping (ONLY CALLABLE DURING INIT STAGE 1)
void INIT_map_section(uint32_t* pt, uint32_t pa, uint32_t va, uint32_t flags) __attribute__((section(".text.init")));

// Creates a kernel page table that covers the top 2GB and a temporary ttb0 to use before
// jumping into high memory (ONLY CALLABLE DURING INIT STAGE 1)
int32_t INIT_kernel_tt_setup(uint32_t* pt, uint32_t* tmp_ttb0) __attribute__((section(".text.init")));

// Creates a section mappin to the given virtual address
void map_section(uint32_t* pt, uint32_t pa, uint32_t va, uint32_t flags);

// Note: we expect pt to point towards section of USR_PT_SIZE allocated bytes
void map_kpage(uint32_t* pt, uint32_t num_lvl1_entries, uint32_t pa, uint32_t va);

// Prints out everything you'd ever want to know about a virtual address to a page and how it's mapped
void mem_print_page_va_info(uint32_t* pt, uint32_t va);

#endif