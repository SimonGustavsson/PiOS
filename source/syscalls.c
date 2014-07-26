#include "interruptHandlers.h"
#include "syscalls.h"
#include "types/string.h"
#include "stddef.h"
#include "scheduler.h"
#include "hardware/uart.h"

void pios_print(const char* str)
{
    // Note: res is not actually used at all
    unsigned int res;
    SWI_PARAM_WITH_RES(SYS_PRINT_SWI, str, res);
}

thread* pios_thread_create(thread_entry entry)
{
    Uart_SendString("pios_create_thread called\n");
    unsigned int res = NULL;
    SWI_PARAM_WITH_RES(SYS_THREAD_CREATE, entry, res);

    return (thread*)res;
}

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

thread* sys_create_thread(int arg1, int arg2, int arg3)
{
    printf("sys_create_thread called from SWI\n");
    return thread_create((thread_entry)arg1);
}

void syscalls_init(void)
{
    swi_install(12, sys_printf);
    swi_install(SYS_THREAD_CREATE, sys_create_thread);
    swi_install(95, sys_dummy1);
    swi_install(96, sys_dummy2);
}
