#include "hardware/timer.h"

volatile timer* gTimer = (timer*)PERIPHERAL_VA_TIMER;

// The timer has 4 channels, we use the forth channel for period interrupts
const unsigned int TIMER_PERIODIC_CHANNEL = 3;

unsigned int Timer_Initialize(void)
{
	// Setup period irq interval before enabling interrupts
	//timer_sp_clearmatch();
	//timer_sp_setinterval(TIMER_INTERRUPT_INTERVAL);
	
	return 0;
}

// Set the interval for the system period timer
void Timer_SetInterval(unsigned int milliSeconds)
{
	unsigned int currentCounter = gTimer->clo;
	
	// Convert to micro seconds
	currentCounter += (milliSeconds * 1000);

	// Note: SP uses channel 3
	gTimer->c3 = currentCounter;
}

void Timer_Clear(void)
{
	// Setting the channels bit in the status registers clears this match
	// (Note: SP uses channel 1)
	gTimer->cs.bits.m3 = 1;
}

// Gets the value of the free running timer
long long Timer_GetTicks(void)
{
    return (gTimer->chi << 32) | gTimer->clo;
}

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
