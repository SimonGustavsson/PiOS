extern unsigned int GET32(unsigned int);
extern void PUT32(unsigned int, unsigned int);

#define SYSTIMER_COUNTER 0x20003004
#define TIMER_BIT 0x00400000unsigned int);

void Wait(unsigned int seconds)
{
	unsigned int ttw = 1048576 * seconds; // 0x00400000
	unsigned int start = GET32(SYSTIMER_COUNTER);
	while(1)
	{
		if(GET32(SYSTIMER_COUNTER) - start >= ttw)
			break;
	}
}
