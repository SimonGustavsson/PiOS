#include "timer.h"

volatile timer* gTimer = (timer*)TIMER_BASE;

// The timer has 4 channels, we use the second channel for period interrupts
const unsigned int TIMER_PERIODIC_CHANNEL = 1;

extern unsigned int GET32(unsigned int);

unsigned int timer_init(void)
{
	// Setup period irq interval before enabling interrupts
	timer_sp_clearmatch();
	timer_sp_setinterval(TIMER_INTERRUPT_INTERVAL);
	
	return 0;
}

// Set the interval for the system period timer
void timer_sp_setinterval(unsigned int interval)
{
	unsigned int currentCounter = gTimer->clo;
	
	currentCounter += interval;

	// Note: SP uses channel 1
	gTimer->c1 = currentCounter;
}

void timer_sp_clearmatch(void)
{
	// Setting the channels bit in the status registers clears this match
	// (Note: SP uses channel 1)
	gTimer->cs.bits.m1 = 1;
}

// TODO: user timer* instead of GET32
void wait(unsigned int milliSeconds)
{
	unsigned int ttw = 1048 * milliSeconds;
	unsigned int start = gTimer->clo;
	while(1)
	{
		if(gTimer->clo - start >= ttw)
			break;
	}
}
