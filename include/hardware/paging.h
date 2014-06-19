#ifndef PAGING_H
#define PAGING_H

#include "memory.h"

// First kernel VA is 0x80000000
// If we take this address: 0x80000000 (_1_0000000000000000000000000000000b)
// If the 31th bit of a VA is set (ie the address is > 2GB) then the kernel's page table is used
// Otherwise the user page table is searched

// "Size" is the size of the address space it covers, value is number of level 1 entries
#define TTB_SIZE_4GB_SIZE   4096
#define TTB_SIZE_2GB_SIZE   2048
#define TTB_SIZE_1GB_SIZE   1024
#define TTB_SIZE_512MB_SIZE  512
#define TTB_SIZE_256MB_SIZE  256
#define TTB_SIZE_128MB_SIZE  128
#define TTB_SIZE_64MB_SIZE    64
#define TTB_SIZE_32MB_SIZE    32

// Values and their meanings for Translation Table Base Control Register 2:0
typedef enum {
    ttbc_16KB     = 0,
    ttbc_8KB      = 1,
    ttbc_4KB      = 2,
    ttbc_2KB      = 3,
    ttbc_1KB      = 4,
    ttbc_512byte  = 5,
    ttbc_256byte  = 6,
    ttbc_128bytes = 7
} ttbc_ttbr0_size;

enum PAGE_TABLE_TYPE{
    PAGE_TABLE_FAULT    = 0x0,
    PAGE_TABLE_COARSE   = 0x1,
    PAGE_TABLE_SECTION  = 0x2,
    PAGE_TABLE_RESERVED = 0x3,
    PAGE_TABLE_MASK     = 0x3 // Masks out the type of the page table
};

// Extended 4KB small pages ARMv6 level 2 descriptor attributes
enum PAGE_ATTR{
    PAGE_FAULT         = 0x0, // Access to page results in translation fault
    PAGE_EXECUTE_NEVER = 0x1, // Page contains no executable code
    PAGE_SMALL         = 0x2, // Small 4KB extended page
    PAGE_BUFFERABLE    = 0x4, // Bufferable
    PAGE_CACHEABLE     = 0x8  // Cacheable
};

// Sets bit 15 (when applicable) and 11:10
enum SECTION_AP {
    SECTION_AP_NONE         = 0x0,   // APX: b0, AP: b00 No access
    SECTION_AP_K_RW         = 0x400, // APX: b0, AP: b01 Kernel Read/Write, User None
    SECTION_AP_K_RW_U_R     = 0x800, // APX: b0, AP: b10 Kernel Read/Write, User Read-Only
    SECTION_AP_K_RW_U_RW    = 0xC00, // APX: b0, AP: b11 Kernel Read/Write, User Read/Write
    SECTION_AP_NONE_DOMAIN  = 0x0,   // APX: b1, AP: b00 Kernel None, domain fault encoded field
    SECTION_AP_K_R          = 0x8400,// APX: b1, AP: b01 Kernel Read-Only, User None
    SECTION_AP_K_R_U_R      = 0x8800 // APX: b1, AP: b10 Kernel Read-Only, User Read-Only
};                    

// Sets bit 6 (when applicable) and 5:4
enum PAGE_AP {
    PAGE_AP_NONE        = 0,    // APX: b0, AP: b00 No access
    PAGE_AP_K_RW        = 0x10, // APX: b0, AP: b01 Kernel Read/Write, User None  
    PAGE_AP_K_RW_U_R    = 0x20, // APX: b0, AP: b10 Kernel Read/Write, User Read Only (Writes in user mode generate permission fault)
    PAGE_AP_K_RW_U_RW   = 0x30, // APX: b0, AP: b11 Kernel Read/Write, User Read/Write
    PAGE_AP_NONE_DOMAIN = 0x40, // APX: b1, AP: b00 Domain fault encoded field
    PAGE_AP_K_R         = 0x50, // APX: b1, AP: b01 Kernel Read Only, User None
    PAGE_AP_K_R_U_R     = 0x60  // APX: b1, AP: b10 Kernel Read Only, User Read Only
};

// Creates a section mapping (ONLY CALLABLE DURING INIT STAGE 1)
void INIT_map_section(unsigned int* pt, unsigned int pa, unsigned int va, unsigned int flags) __attribute__((section(".text.init")));

// Creates a kernel page table that covers the top 2GB and a temporary ttb0 to use before
// jumping into high memory (ONLY CALLABLE DURING INIT STAGE 1)
int INIT_kernel_tt_setup(unsigned int* pt, unsigned int* tmp_ttb0) __attribute__((section(".text.init")));

// Creates a section mappin to the given virtual address
void map_section(unsigned int* pt, unsigned int pa, unsigned int va, unsigned int flags);

// Note: we expect pt to point towards section of USR_PT_SIZE allocated bytes
void map_page(unsigned int* pt, unsigned int num_lvl1_entries, unsigned int pa, unsigned int va, unsigned int flags);

// Prints out everything you'd ever want to know about a virtual address to a page and how it's mapped
void mem_print_page_va_info(unsigned int* pt, unsigned int va);

#endif