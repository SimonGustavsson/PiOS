#include "timer.h"

extern unsigned int GET32(unsigned int);

void timer_setinterval(timer* timer, unsigned int channel, unsigned int interval)
{
	unsigned int currentCounter = timer->clo;
	
	currentCounter += interval;
	
	if(channel == 0)
		timer->c0 = currentCounter;
	else if(channel == 1)
		timer->c1 = currentCounter;
	else if(channel == 2)
		timer->c2 = currentCounter;
	else if(channel == 3)
		timer->c3 = currentCounter;
		
}

void timer_clearmatch(timer* timer, unsigned int channel)
{
	// Set the corresponding bit in cs to 1 to clear that channel
	if(channel == 0)
		timer->cs = 1;
	else if(channel == 1)
		timer->cs = 2;
	else if(channel == 2)
		timer->cs = 4;
	else if(channel == 3)
		timer->cs = 8;
}

// TODO: user timer* instead of GET32
void wait(unsigned int milliSeconds)
{
	unsigned int ttw = 1048 * milliSeconds;
	unsigned int start = GET32(SYSTIMER_COUNTER);
	while(1)
	{
		if(GET32(SYSTIMER_COUNTER) - start >= ttw)
			break;
	}
}
