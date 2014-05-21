#include "memory.h"

#define COARSE_TABLE_DESCRIPTOR 1

// Kernel page table covers the top 2GB of the 4Gb address space
// But the page table is 16k anyway
#define KRL_LEVEL1_ENTRIES 4096

// The level 1 entries are all in sequence, and then all level 2 entries
// are placed at the end of the level 1 entries

// First kernel VA is 0x80000000

// If we take this address: 0x80000000 (_1_0000000000000000000000000000000b)
// If the 31th bit of a VA is set (ie the address is > 2GB) then the kernel's page table is used
// Otherwise the user page table is searched

// Creates a kernel page table for TTB1 that covers the top 2GB
// (N = 1) see: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0211k/Bihgfcgf.html

// Level 1 types
#define PT_TYPE_FAULT 0
#define PT_TYPE_COARSE 1
#define PT_TYPE_SECTION 2
#define PT_TYPE_RESERVED 3

// Level 2 types
#define PT_TYPE_LARGEPAGE 1
#define PT_TYPE_SMALLPAGE 2

#define PAGE_PRESENT (1 << 9)
#define PAGE_AP_SVCRW (1 << 10)
#define PAGE_AP_RO (2 << 10)
#define PAGE_AP_RW (3 << 10)
#define PAGE_BUFFERABLE (1 << 2)
#define PAGE_CACHEABLE (1 << 3)

#define USR_PT_LVL1_ENTRIES (2048)
#define USR_PT_LVL2_ENTRIES_PER_LVL1 (256)

// Amount of bytes required for user mode page table
#define USER_PT_SIZE ((USR_PT_LVL1_ENTRIES * sizeof(int)))

#define KRL_PT_LVL1_ENTRIES (4096)

// Amount of bytes required for SVC mode page table
#define KRL_PT_SIZE (KRL_PT_LVL1_ENTRIES * sizeof(int))

#define SMALLPAGE_AP_RW (2 << 4)
#define SMALLPAGE_EXECUTENEVER 1

// Maps the given virtual address to the given physical address
void kernel_pt_initialize_set(unsigned int* pt, unsigned int pa, unsigned int va, unsigned int flags) __attribute__((section(".text.init")));

void kernel_pt_set(unsigned int* pt, unsigned int pa, unsigned int va, unsigned int flags);

// Initializes the kernel page table. Note: pt and tmp_ttb0 are both expected to be allocated with KRL_PT_SIZE bytes
int kernel_pt_initialize(unsigned int* pt, unsigned int* tmp_ttb0) __attribute__((section(".text.init")));

// Note: we expect pt to point towards section of USR_PT_SIZE allocated bytes
int user_pt_initialize(unsigned int* pt, unsigned int physical_start);