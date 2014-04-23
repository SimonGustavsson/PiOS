//
// Example application Zero
// Test to make sure loaded relocatable binaries can call SVC
// Functions repeatedly and return as expected
//
#define SWI_PARAM_WITH_RES(SWI_NR, ARG, RESULT) \
    asm volatile(\
    "mov r0,%1  \t\n" \
    "swi %a2     \n\t" \
    "mov %0,r0  \n\t" \
    : "=r" (RESULT) : "r" (ARG), "I" (SWI_NR) : "r0", "lr")
	
#define SYS_PRINT_SWI 12
	
// Forward declare
void sys_print(const char* str);

int main(void)
{
	unsigned int i;
	while(1)
	{
		sys_print("Hello, I'm dummy2!\n");
		
		// Wait a bit
		for(i = 0; i < 9999999; i++);
	}
	
	// We don't actually return
	return 4;
}

void sys_print(const char* str)
{
	// Note: res is not actually used at all
	unsigned int res;
	SWI_PARAM_WITH_RES(SYS_PRINT_SWI, str, res);
}


