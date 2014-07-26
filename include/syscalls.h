#include "thread.h"

#define SWI_PARAM_WITH_RES(SWI_NR, ARG, RESULT) \
    asm volatile(\
    "mov r0,%1  \t\n" \
    "swi %a2     \n\t" \
    "mov %0,r0  \n\t" \
    : "=r" (RESULT) : "r" (ARG), "I" (SWI_NR) : "r0", "lr")

#define SYS_PRINT_SWI 12
#define SYS_THREAD_CREATE 42

// Invokes the system call to print a string
void pios_print(const char* str); 
thread* pios_thread_create(thread_entry entry);

void syscalls_init(void);