#include "memory_map.h"

#define BYTES_PER_SLICE 4 
#define MAX_BYTES_PER_SIZE_BYTE ((2^7) - 1)
#define MAX_ALLOCATED_BYTES 104857600 // 100 MB, TODO: Don't hardcode this
#define MAX_ALLOCATED_SLICES (MAX_ALLOCATED_BYTES / BYTES_PER_SLICE) // 26214400
#define EXTENDED_SIZE_BYTE_FLAG (1 << 7)

//#define DEBUG_MEM

// Initializes the allocator
void Pallocator_Initialize(void);

// Allocates 'size' bytes and returns a pointer to the newly allocated memory
void* palloc(unsigned int size);

// Allocates memory for an array of 'size' with elements of 'itemSize'
// Example: pcalloc(sizeof(int), 4); // Allocates an array of 4 ints
void* pcalloc(unsigned int itemSize, unsigned int size);

void phree(void* pointer);

// Resizes a previously allocated chunk of memory and returns a pointer
// to the new chunk. If size is 0, ptr is freed and NULL is returned.
// if ptr is NULL, a new chunk is allocated.
void* realloc(void* ptr, unsigned int size);