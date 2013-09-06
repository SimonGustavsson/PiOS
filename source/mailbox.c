#include "mailbox.h"

static volatile unsigned int *gMailbox0Read = (unsigned int *)(0x2000b880);
static volatile unsigned int *gMailbox0Status = (unsigned int *)(0x2000b898);
static volatile unsigned int *gMailbox0Write = (unsigned int *)(0x2000b8a0);

unsigned int Mailbox_Read(unsigned int channel)
{
	unsigned int count = 0;
	unsigned int data;

	// Loop until something is received on the channel
	while(1)
	{
		while (*gMailbox0Status & MAILBOX_EMPTY)
		{
			// Arbitrary large number for timeout
			if(count++ >(1<<25))
			{
				return 0xffffffff;
			}
		}
		
		data = *gMailbox0Read;

		if ((data & 15) == channel)
			return data;
	}
}

void Mailbox_Write(unsigned int channel, unsigned int data)
{
	// Wait until there's space in the mailbox
	while (*gMailbox0Status & MAILBOX_FULL){
	}
	
	// 28 MSB is data, 4 LSB = channel
	*gMailbox0Write = (data | channel);
}

unsigned int MailboxSetPowerState(unsigned int deviceId, HardwarePowerState state)
{
	volatile unsigned int mailbuffer[256] __attribute__ ((aligned (16)));
	unsigned int mailbufferAddr = (unsigned int)mailbuffer;
	
	mailbuffer[0] = 8 * 4;		// Size of this message
	mailbuffer[1] = 0;			// This is a request
	mailbuffer[2] = 0x00020001;	// Set power state tag
	mailbuffer[3] = 0x8;		// Value buffer size
	mailbuffer[4] = 0x8;		// Value length is 8
	mailbuffer[5] = deviceId;	// Device Id
	mailbuffer[6] = state;		// Set power off
	mailbuffer[7] = 0; 			// Closing tag
	
	Mailbox_Write(8, mailbufferAddr);
	
	Mailbox_Read(8);
	
	// Check if device was found (bit 1 == 0)
	if((mailbuffer[6] & 1) == 1)
		return 0; // Not found
		
	// Make sure that the power bit is what we expect
	if((mailbuffer[6] & 2) == (state & 2))
		return 1;
	else
		return 0;
}
