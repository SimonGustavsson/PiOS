void c_undefined_handler(void* lr);
void c_abort_data_handler(unsigned int address, unsigned int errorType, unsigned int accessedAddr, unsigned int* sp);
void c_abort_instruction_handler(unsigned int address, unsigned int errorType);
void c_swi_handler(unsigned int swi);
void c_irq_handler(volatile unsigned int* r0);
void print_abort_error(unsigned int errorType);