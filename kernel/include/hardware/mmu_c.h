#define PAGE_TABLE_SIZE 4096 // 4 GB, map all of the addressesies
#ifndef UTILITIES_H
#define UTILITIES_H

#define MMU_DEBUG

typedef struct {
	unsigned int sectionTypeIdentifier : 2; // Must be 2 (b10)
	unsigned int bufferable : 1;
	unsigned int cacheable : 1;
	unsigned int executeNever : 1;
	unsigned int domain : 4;
	unsigned int present : 1;
	unsigned int accessPermission : 2;
	unsigned int typeExtension : 3;
	unsigned int apx : 1;
	unsigned int shared : 1;
	unsigned int notGlobal : 1;
	unsigned int mustBeZero : 1;
	unsigned int notSecure : 1;
	unsigned int baseAddress : 12;
} SectionPageTable;

typedef union {
	struct {
		unsigned int sectionTypeIdentifier : 2; // Must be 2 (b10)
		unsigned int bufferable : 1;
		unsigned int cacheable : 1;
		unsigned int executeNever : 1;
		unsigned int domain : 4;
		unsigned int present : 1;
		unsigned int accessPermission : 2;
		unsigned int typeExtension : 3;
		unsigned int apx : 1;
		unsigned int shared : 1;
		unsigned int notGlobal : 1;
		unsigned int mustBeZero : 1;
		unsigned int notSecure : 1;
		unsigned int baseAddress : 12;
	} bits;
	unsigned int raw;
} SectionPageTable2;

typedef enum {
	APNone = 0,
	APNoneSvcRw = 1,
	APReadOnly = 2,
	APReadWrite = 3
} AccessPermission;

// We only store the top 12 bits as that's
// What the page table needs
typedef struct {
	short int virtAddr;
	short int phyAddr;
	unsigned int active;
} MemoryMapping;

typedef struct {
	MemoryMapping* mappings;
	unsigned int* startAddr;
} Pfa;

void Mmu_Initialize(unsigned int*);
void Mmu_MapSection(unsigned int physicalAddressStart, unsigned int virtualAddressStart, 
    unsigned int numSections, unsigned int ap, unsigned int cacheable, unsigned int bufferable);
void Mmu_UnmapSection(unsigned int virtualAddr);

//void mmu_AllocateUserPage(unsigned int physicalAddr, unsigned int virtualAddr);
//void mmu_DeallocateUserPage(unsigned int virtualAddr);

#endif
