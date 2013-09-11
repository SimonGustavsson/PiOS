#include "mailbox.h"
#include "stringutil.h"
#include "timer.h"

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

unsigned int Mailbox_SetDevicePowerState(unsigned int deviceId, unsigned int powerState)
{
	volatile unsigned int mailbuffer[256] __attribute__ ((aligned (16)));

	unsigned int bufSize = 1;
	mailbuffer[bufSize++] = 0;          // 1. Request indicator
	mailbuffer[bufSize++] = 0x00028001; // 2. TAG - Set Power
	mailbuffer[bufSize++] = 0x8;        // 3. Value buffer size (in bytes)
	mailbuffer[bufSize++] = 0x8;        // 4. Value size (in bytes)
	mailbuffer[bufSize++] = deviceId;   // 5. Value - Device id
	mailbuffer[bufSize++] = powerState; // 6. Value - State (On, do not wait)
	mailbuffer[bufSize++] = 0;          // 7. End of message tag
	mailbuffer[0] = bufSize * 4;        // 0. Size of this buffer (in bytes)
	
	Mailbox_Write(8, (unsigned int)mailbuffer);
	
	wait(200); // Wait for device to power on (Note we're passing in "don't wait" to power cmd)
	
	Mailbox_Read(8);
	
	if(mailbuffer[1] != RESPONSE_SUCCESS)
	{
		printf("Failed to change power state of device '%d', Invalid mailbox response.\n", deviceId);
		return -1;
	}
	
	if(mailbuffer[5] != 0x0)
	{
		printf("Failed to change power state of device, invalid device id '%d'.\n", deviceId);
		return -1;
	}
	
	if((mailbuffer[6] & 0x3) != powerState)
	{
		printf("Failed to change power state of device, device did not change state successfully, current state: %d.\n", mailbuffer[6]);
		return -1;
	}
	
	if(powerState == 0)
		printf("'%d' is now OFF.\n", deviceId);
	else
		printf("'%d' is now ON.\n", deviceId);
		
	return 0;
}
