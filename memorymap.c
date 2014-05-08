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
	/* 
		The startup code as per Rpi requirements gets placed at the start of the binary
		So that it can be jumped to. The init section which performs initialization of
		Virtual memory is placed directly afterwards
	*/
    .text.boot : { *(.text.boot*) } > ram
    .text.init : { *(.text.init*) } > ram
	
	/*
		Then place the of the kernel in high memory
	*/
    . = KERNEL_VA_START;
	
    .text : { *(.text*) } > ram
    . = ALIGN(4096); /* align to page size */
	
    .bss : { *(.bss*) } > ram
    . = ALIGN(4096); /* align to page size */
	
	.data : { *(.data) } > ram
	.rodata : { *(.rodata) } > ram
	
	/* TODO: Investigate What these are */
    .dynamic    : { *(.dynamic) } > ram
    .got         : { *(.got) } > ram
    .igot.plt     : { *(.igot.plt) } > ram
    .igot         : { *(.igot) } > ram
    .plt           : { *(.plt) } > ram
    .rel.plt       : { *(.rel.plt) } > ram
    .rel.dyn       : { *(.rel.dyn) } > ram
}