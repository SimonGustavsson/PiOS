#include "util/utilities.h"
#include "memory_map.h"
#include "types/string.h"

#define PM_PASSWORD 0x5a000000
#define PM_RSTC_WRCFG_FULL_RESET 0x00000020

void* my_memcpy(const void *dest, const void *src, unsigned int bytesToCopy)
{
    char *s = (char *)src;
    char *d = (char *)dest;
    while (bytesToCopy > 0)
    {
        *d++ = *s++;
        bytesToCopy--;
    }
    return (void*)dest; // Disregards const modifier
}

void reboot(void)
{
    *(unsigned int*)(PERIPHERAL_VA_WDOG) = (PM_PASSWORD | 1); // timeout = 1/16th of a second? (whatever)
    *(unsigned int*)(PERIPHERAL_VA_RSTC) = (PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);

    // Watch for the watchdog to do its thing
    while (1);
}

void HexDump(void* mem, unsigned int size)
{
    unsigned char* memPtr = (unsigned char*)mem;
    do
    {
        unsigned int i;
        for (i = 0; i < 16; i++)
        {
            char buf[10];
            my_sscanf(&buf[0], "%2X ", *memPtr++);
            printf(buf);

            size--;

            if (size == 0)
                break;
        }

        printf("\n");

    } while (size > 0);
}
