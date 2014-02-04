#include "mmu.h"
#include "stringutil.h"

extern void initSystemPostMmu(void);
extern void enable_mmu_and_cache(unsigned int* pageTableBase);

void initMmu(unsigned int* pageTableBase)
{
	printf("mmu - Initializing page table at 0x%h\n", pageTableBase);

	// Initialize all PTEs to translation fault (0)
	unsigned int i;
	for (i = 0; i < 4096; i++)
		*(pageTableBase + i) = 0; // STMIA?

	printf("Setting up kernel mappings for first 200MB (SVC only).\n");
	printf("Setting up kernel mappings for peripherals at 0x20000000 (SVC only).\n");

	// The first 200 MB is all kernel data (including memory allocator), do a 1:1 mapping
	mmuMapSection(pageTableBase, 0x00000000, 0x00000000, 200, APNoneSvcRw, 1, 1);
	mmuMapSection(pageTableBase, 0x20000000, 0x20000000, 256, APNoneSvcRw, 0, 0); //0xA0000000

	// The remaining 312 MB is all usermode
	// To map user mode (Note that the physical address will probably change based on proc id)
	// mmuMapSection(0x008C00000, 0xB0000000, 5, ReadWrite, true, true);

	enable_mmu_and_cache(pageTableBase);

	printf("mmu - Initializing complete");
}

void mmuMapSection(unsigned int* pageTable, unsigned int physicalAddressStart, unsigned int virtualAddressStart,
	unsigned int numSections, unsigned int ap, unsigned int cacheable, unsigned int bufferable)
{
	// Top 12 bits of virtual address index into the page table directory
	unsigned int vaBase = (virtualAddressStart >> 20) & 0xFFF;
	unsigned int paBase = (physicalAddressStart >> 20) & 0xFFF;

	unsigned int i;
	for (i = 0; i < numSections; i++)
	{
		SectionPageTable* section = (SectionPageTable*)(pageTable + vaBase + i);
		section->baseAddress = paBase + i;
		section->domain = 0; // Hardcoded domain - We don't use domains
		section->present = 1;
		section->cacheable = cacheable;
		section->bufferable = bufferable;
		section->accessPermission = ap;
		section->sectionTypeIdentifier = 2;
	}
}