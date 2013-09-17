#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#define INTERRUPT_BASE 0x2000B200

typedef struct {
	volatile unsigned int Basic;
	volatile unsigned int Pending1;
	volatile unsigned int Pending2;
	volatile unsigned int FiqControl;
	volatile unsigned int Enable1;
	volatile unsigned int Enable2;
	volatile unsigned int BasicEnable;
	volatile unsigned int Disable1;
	volatile unsigned int Disable2;
	volatile unsigned int BasicDisable;
} InterruptRegister;

unsigned int interrupts_init(void);

#endif
