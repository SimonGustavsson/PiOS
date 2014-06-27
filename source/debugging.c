#include "debugging.h"
#include "types/string.h" // printf(...)
#include "asm.h" // get_fp()

extern char _binary_bin_obj_piosfunc_txt_start;
extern char _binary_bin_obj_piosfunc_txt_end;

void Debug_ReadFunctionNames(void)
{
    char* first = (char*)&_binary_bin_obj_piosfunc_txt_start;
    unsigned int blob_end = &_binary_bin_obj_piosfunc_txt_end;

    printf("Binary blog of function names start at 0x%h\n", first);
    printf("Binary blog of function names ends at 0x%h\n", blob_end);
    printf("Value: %s\n", first);
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
