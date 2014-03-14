#include "mmu.h"
#include "stringutil.h"
#include "memory.h"

extern void enable_mmu_and_cache(unsigned int* pageTableBase);

Pfa* gPfa;
unsigned int* gPageTableBase;

void initMmu(unsigned int* pageTableBase)
{
	printf("mmu - Initializing page table at 0x%h\n", pageTableBase);

	gPageTableBase = pageTableBase;

	// Initialize all PTEs to translation fault (0)
	unsigned int i;
	for (i = 0; i < 4096; i++)
		*(pageTableBase + i) = 0; // STMIA?

    printf("mmu - Kernel: 0x000 -> C800000 (200MB)\n");
    printf("mmu - Peripherals: 0x20000000 -> 0x10000000(256MB)\n");

	// The first 200 MB is all kernel data (including memory allocator), do a 1:1 mapping
	mmuMapSection(0x00000000, 0x00000000, 200, APNoneSvcRw, 1, 1);
	mmuMapSection(0x20000000, 0x20000000, 256, APNoneSvcRw, 0, 0); //0xA0000000

	// The remaining 312 MB is all usermode
	// To map user mode (Note that the physical address will probably change based on proc id)
	// mmuMapSection(0x008C00000, 0xB0000000, 5, ReadWrite, true, true);
    mmuMapSection(0x0A827000, 0xC0000000, 5, APReadWrite, 1, 1);

	enable_mmu_and_cache(pageTableBase);

	printf("mmu - Initializing complete\n");
}

void mmuMapSection(unsigned int physicalAddressStart, unsigned int virtualAddressStart,
	unsigned int numSections, unsigned int ap, unsigned int cacheable, unsigned int bufferable)
{
	// Top 12 bits of virtual address index into the page table directory
	unsigned int vaBase = (virtualAddressStart >> 20) & 0xFFF;
	unsigned int paBase = (physicalAddressStart >> 20) & 0xFFF;

	unsigned int i;
	for (i = 0; i < numSections; i++)
	{
		SectionPageTable* section = (SectionPageTable*)(gPageTableBase + vaBase + i);
		section->baseAddress = paBase + i;
		section->domain = 0; // Hardcoded domain - We don't use domains
		section->present = 1;
		section->cacheable = cacheable;
		section->bufferable = bufferable;
		section->accessPermission = ap;
		section->sectionTypeIdentifier = 2;
	}
}

void mmuUnmapSection(unsigned int virtualAddr)
{
	// Reset the page completely
	*(gPageTableBase + ((virtualAddr >> 20) & 0xFFF)) = 0;
}

// Allocate shouldn't tkae in address - it should just give the caller an addr
// from the pool and return a MemoryMapping describing it
//void mmu_AllocateUserPage(unsigned int physicalAddr, unsigned int virtualAddr)
//{
//	unsigned int physicalBase = (physicalAddr >> 20) & 0xFFF;
//	unsigned int virtualBase = (virtualAddr >> 20) & 0xFFF;
//	
//	mmuMapSection(physicalAddr, virtualAddr, 1, APReadWrite, 1, 1);
//
//	gPfa->mappings[virtualBase].phyAddr = physicalAddr;
//}
//
//void mmu_DeallocateUserPage(unsigned int physAddr)
//{
//	unsigned int physAddrBase = (physAddr >> 20) & 0xFFF;
//
//	MemoryMapping* mapping = &gPfa->mappings[physAddrBase];
//
//	mapping->active = 0;
//	mapping->virtAddr = 0;
//}
