#include "debugging.h"
#include "types/string.h" // printf(...)
#include "asm.h" // get_fp()

void Debug_PrintCallstack(void)
{
    printf("Stack trace: ");
    int lr = 0;
    int depth = 0;
    int* fp = get_fp();

    printf("From: 0x%h -> ", (int)fp);
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
