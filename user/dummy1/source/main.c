#define SVC_INSTRUCTION(number) asm volatile("svc %0" : : "I" (number))

//
// Example application Zero
// Test to make sure loaded relocatable binaries can call SVC
// Functions repeatedly and return as expected
//
int main(void)
{
	unsigned int i;
	for(i = 0; i < 4; i++)
		SVC_INSTRUCTION(12);
	
	return 0;
}
