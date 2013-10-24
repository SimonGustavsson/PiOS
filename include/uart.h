#define UART_BASE 0x2020100

typedef volatile struct{
	unsigned int dr;   // Data Register
	unsigned int rsrecr; // ?
	unsigned int reserved[4]; 
	unsigned int fr;   // Flag Register
	unsigned int ilpr; // Not used
	unsigned int ibrd; // Integer baud rate divisor
	unsigned int fbrd; // Fractional baud rate divisor
	unsigned int lcrh; // Line control register
	unsigned int cr;   // Control Register
	unsigned int ifls; // Interrupt FIFO Level Select Register
	unsigned int imsc; // Interrupt mask set clear register
	unsigned int ris;  // Raw interrupt status register
	unsigned int mis;  // Masked interrupt status register
	unsigned int icr;  // Interrupt clear Register
	unsigned int dmacr;// DMA control register
	unsigned int itcr; // Test control register
	unsigned int itip; // Integration test input reg
	unsigned int itop; // Integration test output reg
	unsigned int tdr;  // Test data reg
} Uart;

unsigned int uart_initialize(void);
void uart_send(unsigned int);
unsigned int uart_read();
void uart_send_string(char*);
