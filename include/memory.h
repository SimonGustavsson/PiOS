#include "memory_map.h"

#define BYTES_PER_SLICE 4 
#define MAX_BYTES_PER_SIZE_BYTE ((2^7) - 1)
#define MAX_ALLOCATED_BYTES 104857600 // 100 MB, TODO: Don't hardcode this
#define MAX_ALLOCATED_SLICES (MAX_ALLOCATED_BYTES / BYTES_PER_SLICE) // 26214400
#define EXTENDED_SIZE_BYTE_FLAG (1 << 7)

//#define DEBUG_MEM

void DataMemoryBarrier(void);
void DataSyncBarrier(void);
void FlushCache(void);
void FlushTLB(unsigned int mva);

void Pallocator_Initialize(void);
void* palloc(unsigned int size);

// Allocates memory for an array of 'size' with elements of 'itemSize'
// Example: pcalloc(sizeof(int), 4); // Allocates an array of 4 ints
void* pcalloc(unsigned int itemSize, unsigned int size);

void* my_memset(void* dest, unsigned char c, unsigned int size);

void phree(void* pointer);
