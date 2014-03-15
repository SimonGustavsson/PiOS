#include "util/utilities.h"

#ifdef __cplusplus
extern "C" {
#endif

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
        static const int PM_RSTC = 0x2010001c;
        static const int PM_WDOG = 0x20100024;
        static const int PM_PASSWORD = 0x5a000000;
        static const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;

        *(unsigned int*)(PM_WDOG) = (PM_PASSWORD | 1); // timeout = 1/16th of a second? (whatever)
        *(unsigned int*)(PM_RSTC) = (PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);

        while (1);
    }

#ifdef __cplusplus
}
#endif