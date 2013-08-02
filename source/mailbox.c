extern unsigned int GET32(unsigned int);
extern void PUT32(unsigned int, unsigned int);

#define MAILBOX_FULL 0x80000000
#define MAILBOX_EMPTY 0x40000000

static volatile unsigned int *MAILBOX0READ = (unsigned int *)(0x2000b880);
static volatile unsigned int *MAILBOX0STATUS = (unsigned int *)(0x2000b898);
static volatile unsigned int *MAILBOX0WRITE = (unsigned int *)(0x2000b8a0);

unsigned int Mailbox_Read(unsigned int channel)
{
	unsigned int count = 0;
	unsigned int data;

	/* Loop until something is received from channel
	 * If nothing recieved, it eventually give up and returns 0xffffffff
	 */
	while(1)
	{
		while (*MAILBOX0STATUS & MAILBOX_EMPTY)
		{
			/* This is an arbritarily large number */
			if(count++ >(1<<25))
			{
				return 0xffffffff;
			}
		}
		/* Read the data */
		data = *MAILBOX0READ;

		if ((data & 15) == channel)
			return data;
	}
}

void Mailbox_Write(unsigned int channel, unsigned int data)
{
	/* Wait for mailbox to be not full */
	while (*MAILBOX0STATUS & MAILBOX_FULL){
	}

	*MAILBOX0WRITE = (data | channel);
}