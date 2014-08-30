#include "util/memutil.h"

void* my_memset(void* dest, unsigned char c, unsigned int size)
{
    unsigned char* ptr = (unsigned char*)dest;

    while (size--)
        *ptr++ = c;

    return dest;
}

void DataMemoryBarrier(void)
{
    asm volatile ("mcr p15, #0, %[zero], c7, c10, #5" : : [zero] "r" (0));
}

void DataSyncBarrier(void)
{
    asm volatile ("mcr p15, #0, %[zero], c7, c10, #4" : : [zero] "r" (0));
}

void FlushCache(void)
{
    asm volatile ("mcr p15, #0, %[zero], c7, c14, #0" : : [zero] "r" (0));
}

void InvalidateAllUnlockedTLB(void)
{
    asm volatile ("mcr p15,0, %[zero],c8, c5, 0" : : [zero] "r" (0));
}

void FlushTLB(unsigned int mva)
{
    // c5 = Instruction TLB
    // c6 = Data TLB
    // c7 = Unified TLB
    // TODO: Make generic and take in arg which to clear
    asm volatile ("MCR P15, 0, r0, c8, c6, 1" : : [mva] "r" (mva));
}
