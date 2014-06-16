#ifndef MEMUTIL_H
#define MEMUTIL_H

void DataMemoryBarrier(void);
void DataSyncBarrier(void);
void FlushCache(void);
void FlushTLB(unsigned int mva);
void InvalidateAllUnlockedTLB(void);
void* my_memset(void* dest, unsigned char c, unsigned int size);

#endif