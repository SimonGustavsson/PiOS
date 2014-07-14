#include "interruptHandlers.h"
#include "syscalls.h"
#include "types/string.h"
#include "stddef.h"

void* sys_printf(int arg1, int arg2, int arg3)
{
    char* s = (char*)arg1;

    printf(s);

    return NULL;
}

void* sys_dummy1(int arg1, int arg2, int arg3)
{
    printf("Dummy1 swi called\n");

    return NULL;
}

void* sys_dummy2(int arg1, int arg2, int arg3)
{
    printf("Dummy1 swi called\n");

    return NULL;
}

void syscalls_init(void)
{
    swi_install(12, sys_printf);
    swi_install(95, sys_dummy1);
    swi_install(96, sys_dummy2);
}
