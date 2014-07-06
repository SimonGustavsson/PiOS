#include "atags.h"
#include "types/string.h"

void atags_parse(int* addr)
{
    atag* tag = (atag*)addr;

    while (tag->hdr.tag != ATAG_NONE)
    {
        switch (tag->hdr.tag)
        {
        case ATAG_CORE:
            if (tag->hdr.size == 5) // Has data
                printf("ATAG_CORE - Flags: %d Page Size: %d Root dev: %d\n", tag->u.core.flags, tag->u.core.pagesize, tag->u.core.rootdev);
            else
                printf("ATAG_CORE - No data\n");
            break;
        case ATAG_MEM:
            printf("ATAG_MEM\n");
            break;
        case ATAG_VIDEOTEXT:
            printf("ATAG_VIDEOTEXT\n");
            break;
        case ATAG_RAMDISK:
            printf("ATAG_RAMDISK\n");
            break;
        case ATAG_INITRD2:
            printf("ATAG_INITRD2\n");
            break;
        case ATAG_SERIAL:
            printf("ATAG_SERIAL\n");
            break;
        case ATAG_REVISION:
            printf("ATAG_REVISION\n");
            break;
        case ATAG_VIDEOLFB:
            printf("ATAG_VIDEOLFB\n");
            break;
        case ATAG_CMDLINE:
            printf("ATAG_CMDLINE\n");
            break;
        }

        // Go to next tag
        tag = (atag*)( ((unsigned int)tag) + tag->hdr.size);
    }
}