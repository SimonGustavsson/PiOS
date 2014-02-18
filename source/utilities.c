#ifdef __cplusplus
extern "C" {
#endif

#include "utilities.h"

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

#ifdef __cplusplus
}
#endif