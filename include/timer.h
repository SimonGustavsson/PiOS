
#ifndef TIMER_H
#define TIMER_H

#define SYSTIMER_COUNTER 0x20003004
#define TIMER_BASE 0x20003000 // Bus: 0x7E003000

// TODO: Replace cs in timer struct with bit field below
typedef struct {
	unsigned char m0:1; // 
 	unsigned char m1:1; // | 0 = No timer x match since last clear
	unsigned char m2:1; // | 1 = Timer x match detected
	unsigned char m3:1; //  
} timer_cs_register;

typedef struct {
	unsigned int cs;       // Control/Status
	unsigned int clo;      // Free running counter Lower 32 bits (Read-only)
	unsigned int chi;      // Free running counter higher 32 bits (Read-only)
	unsigned int c0;       // Compare 0 
	unsigned int c1;       // Compare 1  | These 4 registers holds the compare value for each channel.
	unsigned int c2;       // Compare 2  | When a match is found, the corresponding bit is set in cs
	unsigned int c3;       // Compare 3 /
} timer;

void timer_setinterval(timer* timer, unsigned int channel, unsigned int interval);
void timer_clearmatch(timer* timer, unsigned int channel);
void wait(unsigned int milliSeconds);

#endif
