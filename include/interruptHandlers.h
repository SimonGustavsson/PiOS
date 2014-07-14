#include "stdbool.h"

#define MAX_SWI_NUM 200
typedef void*(*swi_handler)(int, int, int);

void c_undefined_handler(void* lr);
void c_abort_data_handler(unsigned int address, unsigned int errorType, unsigned int accessedAddr, unsigned int fault_reg);
void c_abort_instruction_handler(unsigned int address, unsigned int errorType); 
void c_swi_handler(unsigned int r0, unsigned int r1, unsigned int r2, unsigned int swi);
void c_irq_handler(volatile unsigned int* sp);
void print_abort_error(unsigned int errorType);

bool swi_install(unsigned int swiNum, swi_handler handler);