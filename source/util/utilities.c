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

static void swap(void *x, void *y, unsigned int l) {
    char *a = x, *b = y, c;
    while (l--) {
        c = *a;
        *a++ = *b;
        *b++ = c;
    }
}

// sort() shamelessly copied from Wikipedia because I'm tired
// TODO: Look into better ways to do this?
static void sort(char *array, unsigned int size, int(*cmp)(void*, void*), int begin, int end) {
    if (end > begin) {
        void *pivot = array + begin;
        int l = begin + size;
        int r = end;
        while (l < r) {
            if (cmp(array + l, pivot) <= 0) {
                l += size;
            }
            else if (cmp(array + r, pivot) > 0)  {
                r -= size;
            }
            else if (l < r) {
                swap(array + l, array + r, size);
            }
        }
        l -= size;
        swap(array + begin, array + l, size);
        sort(array, size, cmp, begin, l);
        sort(array, size, cmp, r, end);
    }
}

void qsort(void* base, unsigned int nitems, unsigned int element_size, int(*comparer)(const void*, const void*))
{
    sort(base, element_size, comparer, 0, (nitems - 1)*element_size);
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
