#include "memory_map.h"

/*
	Note the .c extension of this file, this is so that we can run it through the C preprocessor
	Which allows us to specify all memory related constants in one place instead of having to dig
	Through multiple files
*/

MEMORY
{
	/* Hardcoded 1 MB kernel size */
    ram : ORIGIN = LD_KRNL_ORI, LENGTH = LD_KRNL_LEN
}

SECTIONS
{
	.text.boot : { *(.text.boot*) } > ram
    .text : { *(.text*) } > ram
    . = ALIGN(4096); /* align to page size */
	
    .bss : { *(.bss*) } > ram
    . = ALIGN(4096); /* align to page size */
	
	.data : { *(.data) } > ram
	.rodata : { *(.rodata) } > ram
}