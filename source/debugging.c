#include "debugging.h"
#include "types/string.h" // printf(...)
#include "asm.h" // get_fp()
#include "memory.h"
#include "util/utilities.h"

extern unsigned int LNK_KERNEL_END;

func_info* gFunctions;

void Debug_ReadFunctionNames(void)
{
    unsigned int kernel_va_end = &LNK_KERNEL_END;

    char* blob = (char*)(kernel_va_end);

    printf("Reading debugging symbols from 0x%h\n", blob);

    unsigned int num_funcs = (blob[0] << 24) | (blob[1] << 16) | (blob[2] << 8) | blob[3];
    blob += 5;

    gFunctions = (func_info*)palloc(sizeof(func_info) * num_funcs);

    unsigned int i;
    for (i = 0; i < num_funcs; i++)
    {
        func_info* cur = &gFunctions[i];

        int nameLen = my_strlen(blob);
        
        cur->name = (char*)palloc(nameLen + 1);
        my_memcpy(cur->name, blob, nameLen);
        cur->name[nameLen] = 0;
        
        blob += nameLen + 1;

        cur->address = (blob[0] << 24) | (blob[1] << 16) | (blob[2] << 8) | (blob[3]);

        //printf("Found '%s' at 0x%h\n", cur->name, cur->address);

        blob += 4;
    }

    printf("Loaded %d function names\n", num_funcs);
}

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
