#define BYTES_PER_SLICE 4 
#define MAX_BYTES_PER_SIZE_BYTE ((2^7) - 1)
#define MAX_ALLOCATED_BYTES 104857600 // 100 MB, TODO: Don't hardcode this
#define MAX_ALLOCATED_SLICES (MAX_ALLOCATED_BYTES / BYTES_PER_SLICE)
#define EXTENDED_SIZE_BYTE_FLAG (1 << 7)

//#define DEBUG_MEM

void DataMemoryBarrier(void);
void DataSyncBarrier(void);
void FlushCache(void);

void Pallocator_Initialize(void);
void* palloc(unsigned int size);
void phree(void* pointer);
