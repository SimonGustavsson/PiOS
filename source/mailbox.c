#define MAILBOX_FULL 0x80000000
#define MAILBOX_EMPTY 0x40000000

static volatile unsigned int *MAILBOX0READ = (unsigned int *)(0x2000b880);
static volatile unsigned int *MAILBOX0STATUS = (unsigned int *)(0x2000b898);
static volatile unsigned int *MAILBOX0WRITE = (unsigned int *)(0x2000b8a0);

unsigned int Mailbox_Read(unsigned int channel)
{
	unsigned int count = 0;
	unsigned int data;

	// Loop until something is received on the channel
	while(1)
	{
		while (*MAILBOX0STATUS & MAILBOX_EMPTY)
		{
			// Arbitrary large number for timeout
			if(count++ >(1<<25))
			{
				return 0xffffffff;
			}
		}
		
		data = *MAILBOX0READ;

		if ((data & 15) == channel)
			return data;
	}
}

void Mailbox_Write(unsigned int channel, unsigned int data)
{
	// Wait until there's space in the mailbox
	while (*MAILBOX0STATUS & MAILBOX_FULL){
	}
	
	// 28 MSB is data, 4 LSB = channel
	*MAILBOX0WRITE = (data | channel);
}

unsigned int MailboxSetPowerState(unsigned int deviceId, HardwarePowerState state)
{
	volatile unsigned int mailbuffer[256] __attribute__ ((aligned (16)));
	unsigned int mailbufferAddr = (unsigned int)mailbuffer;
	
	mailbufferAddr[0] = 8 * 4;		// Size of this message
	mailbufferAddr[1] = 0;			// This is a request
	mailbufferAddr[2] = 0x00020001;	// Set power state tag
	mailbufferAddr[3] = 0x8;		// Value buffer size
	mailbufferAddr[4] = 0x8;		// Value length is 8
	mailbufferAddr[5] = deviceId;	// Device Id
	mailbufferAddr[6] = state;		// Set power off
	mailbufferAddr[7] = 0; 			// Closing tag
	
	Mailbox_Write(8, mailbufferAddr);
	
	Mailbox_Read(8);
	
	// Check if device was found (bit 1 == 0)
	if((mailbufferAddr[6] & 1) == 1)
		return 0; // Not found
		
	// Make sure that the power bit is what we expect
	if((mailbufferAddr[6] & 2) == (state & 2))
		return 1;
	else
		return 0;
}
