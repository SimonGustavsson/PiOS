#include "memory_map.h"

/*
	Note the .c extension of this file, this is so that we can run it through the C preprocessor
	Which allows us to specify all memory related constants in one place instead of having to dig
	Through multiple files
*/

ENTRY(reset)
 
SECTIONS
{
	/* 
		The startup code as per Rpi requirements gets placed at the start of the binary
		So that it can be jumped to. The init section which performs initialization of
		Virtual memory is placed directly afterwards
	*/
    . = LD_KRNL_ORIGIN;
    .text.boot LD_KRNL_ORIGIN : { *(.text.boot*) }
    .text.init LD_KRNL_ORIGIN + SIZEOF(.text.boot) : { *(.text.init*) }
	
	/*
		Set the VMA of the remaining part of the kernel to
        be in high memory as this code runs after the MMU has been turned on
	*/
    . = KERNEL_VA_START + ADDR(.text.init) + SIZEOF(.text.init);
	
    .text . : AT(ADDR(.text.init) + SIZEOF(.text.init)) { *(.text*) }
    . = ALIGN(LD_PAGE_SIZE); /* align to page size */
	 
    .bss : { *(.bss*) }
    . = ALIGN(LD_PAGE_SIZE); /* align to page size */
	
    .data . : { *(.data) }
    .rodata . : { *(.rodata) }
}