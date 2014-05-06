/* uart.h - UART initialization & communication */

#ifndef UART_H
#define UART_H

typedef volatile struct{
	unsigned int dr;           // 0x00 Data Register
	unsigned int rsrecr;       // 0x04 ?
	unsigned int reserved[4];  // -
	unsigned int fr;           // 0x18 Flag Register
	unsigned int reserved2;    // -
	unsigned int ilpr;         // 0x20 Not used
	unsigned int ibrd;         // 0x24 Integer baud rate divisor
	unsigned int fbrd;         // 0x28 Fractional baud rate divisor
	unsigned int lcrh;         // 0x2C Line control register
	unsigned int cr;           // 0x30 Control Register
	unsigned int ifls;         // 0x34 Interrupt FIFO Level Select Register
	unsigned int imsc;         // 0x38 Interrupt mask set clear register
	unsigned int ris;          // 0x3C Raw interrupt status register
	unsigned int mis;          // 0x40 Masked interrupt status register
	unsigned int icr;          // 0x44 Interrupt clear Register
	unsigned int dmacr;        // 0x48 DMA control register
	unsigned int reserved3[13];// -
	unsigned int itcr;         // 0x80 Test control register
	unsigned int itip;         // 0x84 Integration test input reg
	unsigned int itop;         // 0x88 Integration test output reg
	unsigned int tdr;          // 0x8C Test data reg
} Uart;

void Uart_Initialize();
void Uart_EnableInterrupts(void);
void Uart_Send(unsigned char byte);
unsigned char Uart_Read();
void Uart_SendString(const char *str);

#endif // #ifndef UART_H