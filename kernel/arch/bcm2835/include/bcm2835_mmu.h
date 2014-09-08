#ifndef PAGING_H
#define PAGING_H

#include "memory.h"

// First kernel VA is 0x80000000
// If we take this address: 0x80000000 (_1_0000000000000000000000000000000b)
// If the 31th bit of a VA is set (ie the address is > 2GB) then the kernel's page table is used
// Otherwise the user page table is searched

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

#endif