/*
Runtime memory map manager
This allocator works in 4KB chunks, any and all pages allocated/freed by
it will be 4 KB in size. Though it works in page-sized chunks, the memory
addresses handled by this allocator are PHYSICAL.

The pfa_Reserve* functions are meant to be permanent, so they are not expected
to be freed, because of this, the only thing returned is a status code.
ONLY important kernel memory that is never expected to move  should be reserved
(i.e kernel code, framebufferetc).

NOTE: Peripherals need not be mapped as they are outside the physical memory range
*/

#include "mem.h"
#include "memory.h"
#include "types/string.h"

// TODO: Make this a bitmap instead of int array
int* gPages;

static inline unsigned int pfa_GetFirstAddrOfPage(unsigned int pageIndex)
{
    return pageIndex * PAGE_SIZE;
}

static inline unsigned int pfa_GetPageIndexOfAddr(unsigned int addr)
{
    return addr / PAGE_SIZE;
}

int mem_init(void)
{
    gPages = (int*)pcalloc(sizeof(int), MAX_ALLOCATED_PAGES);

    return 0;
}

// Returns the next available page
int mem_nextFree(void)
{
    unsigned int i;
    for (i = 0; i < MAX_ALLOCATED_PAGES; i++)
    {
        if (gPages[i] == 0)
        {
            // Mark as used
            gPages[i] = 1;

#ifdef PAGEMEM_DEBUG
            printf("Found free page %d\n", i);
#endif

            return pfa_GetFirstAddrOfPage(i);
        }
    }

    return -1;
}

int mem_nextFreeContiguous(unsigned int num_pages)
{
    int start_page = -1;

    unsigned int i;
    unsigned int contiguous_found = 0;
    for (i = 0; i < MAX_ALLOCATED_PAGES; i++)
    {
        if (gPages[i] == 0)
        {
            if (start_page == -1)
                start_page = i;

            contiguous_found++;
        }
        else
        {
            contiguous_found = 0;
        }

        if (start_page != -1 && contiguous_found == num_pages)
        {
            for (i = 0; i < num_pages; i++)
                gPages[start_page + i] = 1;

            return pfa_GetFirstAddrOfPage(start_page);
        }
    }

    return -1;
}

// Frees the page that covers the given physical address
void mem_free(unsigned int addr)
{
    unsigned pageIndex = pfa_GetPageIndexOfAddr(addr);

    // Mark as unused, tadaaa!
    gPages[pageIndex] = 0;

#ifdef PAGEMEM_DEBUG
    printf("Freeing page %d\n", pageIndex);
#endif

}

// Marks the page that covers addr as used
int mem_reserveSingle(unsigned int addr)
{
    unsigned int pageIndex = pfa_GetPageIndexOfAddr(addr);

    if (gPages[pageIndex] == 1)
        return -1;

    gPages[pageIndex] = 1;

#ifdef PAGEMEM_DEBUG
    printf("Reserving page %d\n", pageIndex);
#endif

    return 0;
}

int mem_reserveRange(unsigned int startAddr, unsigned int endAddr)
{
    if (startAddr > endAddr)
    {
        printf("Invalid call to pfa_ReserveRange, startAddr(%d) must be lower than endAddr(%d)\n", startAddr, endAddr);
        return -1;
    }

    unsigned int startIndex = pfa_GetPageIndexOfAddr(startAddr);
    unsigned int endIndex = pfa_GetPageIndexOfAddr(endAddr);

    // First make sure they're all available
    unsigned int i = startIndex;
    for (; i <= endIndex; i++)
    {
        if (gPages[i] == 1)
        {
            printf("Ooops! Trying to reserve page '%d' in pfa_ReserveRange that's already allocated!\n", i);
            return -1;
        }
    }

    // Actually allocate the pages
    for (i = startIndex; i <= endIndex; i++)
        gPages[i] = 1;

#ifdef PAGEMEM_DEBUG
    printf("Reserved pages %d->%d\n", startIndex, endIndex);
#endif

    return 0;
}

int mem_reserve(unsigned int startAddr, unsigned int size)
{
    if (size == 0) return -1;

    unsigned int endAddr = startAddr + size;

    return mem_reserveRange(startAddr, endAddr);
}

unsigned int mem_getAllocated(void)
{
    unsigned int allocatedPages = 0;
    unsigned int i;
    for (i = 0; i < MAX_ALLOCATED_PAGES; i++)
    {
        if (gPages[i] == 1) allocatedPages++;
    }

    return allocatedPages * PAGE_SIZE;
}

unsigned int mem_getAvailable(void)
{
    int allocatedMemory = mem_getAllocated();

    return MAX_AVAILABLE_MEMORY - allocatedMemory;
}

void mem_printUsage(void)
{
    unsigned int allocated = mem_getAllocated();

    if (allocated > 1048576) // Show MB
    {
        printf("mem stat: %d/%d mB allocated\n", allocated / 1024 / 1024, MAX_AVAILABLE_MEMORY / 1024 / 1024);
    }
    else if (allocated > 1024) // Show KB
    {
        printf("mem stat: %d/%d kB allocated\n", allocated / 1024, MAX_AVAILABLE_MEMORY / 1024);
    }
    else // Bytes
    {
        printf("mem stat: %d/%d bytes allocated\n", allocated, MAX_AVAILABLE_MEMORY);
    }
}
