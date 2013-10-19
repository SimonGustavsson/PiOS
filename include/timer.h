#ifndef TIMER_H
#define TIMER_H

#define SYSTIMER_COUNTER 0x20003004
#define TIMER_BASE 0x20003000 // Bus: 0x7E003000
#define TIMER_INTERRUPT_INTERVAL 0x00200000

typedef struct {
	volatile union{
		unsigned int raw;
		struct {
			unsigned int m0 : 1;
			unsigned int m1 : 1; // 0 = No timer x match since last clear
			unsigned int m2 : 1; // 1 = Timer x match detected
			unsigned int m3 : 1; 
			unsigned int reserved : 28;
		} bits;
	} cs;
	volatile unsigned int clo;      // Free running counter Lower 32 bits (Read-only)
	volatile unsigned int chi;      // Free running counter higher 32 bits (Read-only)
	volatile unsigned int c0;       // Compare 0 
	volatile unsigned int c1;       // Compare 1  | These 4 registers holds the compare value for each channel.
	volatile unsigned int c2;       // Compare 2  | When a match is found, the corresponding bit is set in cs
	volatile unsigned int c3;       // Compare 3 /
} timer;

unsigned int timer_init(void);
void timer_sp_setinterval(unsigned int interval);
void timer_sp_clearmatch(void);
void wait(unsigned int milliSeconds);

#endif
