
// Sets up the MMU and maps the kernel into high memory
void sysinit_stage1(int, int, int) __attribute__((section(".text.init")));

// Performs initialization of the peripherals and calls cmain

void sysinit_stage2(int, int, int);