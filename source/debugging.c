#include "debugging.h"
#include "string.h" // printf(...)

extern int* get_frame_pointer(void); // defined in start.s

void Debug_PrintCallstack(void)
{
    printf("Stack trace: ");
    int lr = 0;
    int depth = 0;
    int* fp = get_frame_pointer();

    printf("0x%h.\n", (int)fp);
    do
    {
        if ((int)fp == 0 || (int)fp > 0x0A827000)
            break;

        lr = *fp;
        fp = (int*)*(fp - 1);

        printf("0x%h -> ", lr);

    } while (fp != 0 && depth++ < 3 && lr != 0x80CC); // Address of branch to cmain from asm

    printf("\n");
}

void debugDumpStack(unsigned int* sp)
{
    printf("~~~ Stack dump~~~~");

    unsigned int i;
    for (i = 0; i < 40; i++){
        printf("0x%h ", *(sp + i));

        if (i % 10 == 0){
            printf("\n");
        }
    }
}
