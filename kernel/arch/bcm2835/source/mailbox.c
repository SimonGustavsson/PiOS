#include "bcm2835_mailbox.h"
#include "memory.h"
#include "memory_map.h"
#include "types/string.h"
#include "timer.h"
#include "util/memutil.h"

static volatile unsigned int *gMailbox0Read =   (unsigned int *)(PERIPHERAL_VA_MAILBOX_BASE);
static volatile unsigned int *gMailbox0Status = (unsigned int *)(PERIPHERAL_VA_MAILBOX_BASE + 0x18);
static volatile unsigned int *gMailbox0Write =  (unsigned int *)(PERIPHERAL_VA_MAILBOX_BASE + 0x20);

unsigned int Mailbox_Read(unsigned int channel)
{
	unsigned int count = 0;
	unsigned int data;

	// Loop until something is received on the channel
	while(1)
	{
		while (*gMailbox0Status & MAILBOX_EMPTY)
		{
			FlushCache();

			// Arbitrary large number for timeout
			if(count++ >(1<<25))
			{
				return 0xffffffff;
			}
		}
		DataMemoryBarrier();
		
		data = *gMailbox0Read;

		DataMemoryBarrier();

		if ((data & 15) == channel)
			return data;
	}
}

void Mailbox_Write(unsigned int channel, unsigned int data)
{
	// Wait until there's space in the mailbox
	while (*gMailbox0Status & MAILBOX_FULL)
	{
		FlushCache();
	}
	
	DataMemoryBarrier();

	// 28 MSB is data, 4 LSB = channel
	*gMailbox0Write = (data | channel);
}

unsigned int Mailbox_SdGetBaseFrequency(void)
{
	volatile unsigned int mailbuffer[256] __attribute__ ((aligned (16)));
	
	unsigned int bufSize = 1;
	mailbuffer[bufSize++] = 0;          // 1. This is a request
	mailbuffer[bufSize++] = 0x00030002; // 2. "Get clock rate" tag
	mailbuffer[bufSize++] = 8;          // 3. Value buffer size
	mailbuffer[bufSize++] = 4;          // 4. Value length
	mailbuffer[bufSize++] = 1;          // 5. Clock id + response - Clock id
	mailbuffer[bufSize++] = 0;          // 6.  - Response rate (in Hz)
	mailbuffer[bufSize++] = 0;          // 7. Closing tag
	mailbuffer[0] = bufSize * 4;        // 0. Size of the entire buffer (in bytes)

	unsigned int bufferAddr = (unsigned int)&mailbuffer;
	bufferAddr -= KERNEL_VA_START;

	Mailbox_Write(8, bufferAddr);

	Mailbox_Read(8);

	// Check valid response
	if(mailbuffer[1] != RESPONSE_SUCCESS)
	{
		printf("Failed to retrieve SD base frequency - Invalid mailbox response.\n");
		return -1;
	}

	// Check returned clock id
	if(mailbuffer[5] != 0x1)
	{
		printf("Failed to retrieve SD base frequency - Invalid clock id.\n");
		return -1;
	}

	// Return the rate
	return mailbuffer[6];
}

unsigned int Mailbox_GetPowerState(unsigned int deviceId)
{
	volatile unsigned int mailbuffer[256] __attribute__ ((aligned (16)));
	
	unsigned int bufSize = 1;
	mailbuffer[bufSize++] = 0;          // 1. Request indicator
	mailbuffer[bufSize++] = 0x00020001; // 2. Tag - Get power state
	mailbuffer[bufSize++] = 0x8;        // 3. Value buffer size (in bytes)
	mailbuffer[bufSize++] = 0x4;        // 4. Value size (in bytes)
	mailbuffer[bufSize++] = deviceId;   // 5. Value - Device Id
	mailbuffer[bufSize++] = 0;          // 6. Response - State of device
	mailbuffer[bufSize++] = 0;          // 7. End of message tag
	mailbuffer[0] = bufSize * 4;        // 0. Size of entire buffer (in bytes)
	
	unsigned int bufferAddr = (unsigned int)&mailbuffer;
	bufferAddr -= KERNEL_VA_START;

	Mailbox_Write(8, bufferAddr);

	Mailbox_Read(8);
	
	if(mailbuffer[1] != RESPONSE_SUCCESS)
	{
		printf("Failed to get power state, invalid mailbox response");
		return -1;
	}
	
	return mailbuffer[6];
}

int Mailbox_GetClockRate(unsigned int clockId)
{
    volatile unsigned int mailbuffer[256] __attribute__((aligned(16)));

    unsigned int bufSize = 1;
    mailbuffer[bufSize++] = 0;                 // 1. Indicates Requests
    mailbuffer[bufSize++] = MBT_GET_CLOCKRATE; // 2. Tag
    mailbuffer[bufSize++] = 0x8;               // 3. Value buffer size
    mailbuffer[bufSize++] = 0x4;               // 4. Value size
    mailbuffer[bufSize++] = clockId;           // 5. Value - Clock Id
    mailbuffer[bufSize++] = 0;                 // 6. Response - Rate (in Hz)
    mailbuffer[bufSize++] = 0;                 // 7. End of message tag
    mailbuffer[0] = bufSize * 4;               // 0. Size o entire buffer (in bytes)

	unsigned int bufferAddr = (unsigned int)&mailbuffer;
	bufferAddr -= KERNEL_VA_START;

	Mailbox_Write(8, bufferAddr);

    Mailbox_Read(8);

    if (mailbuffer[1] != RESPONSE_SUCCESS)
    {
        printf("Mailbox: Failed to retrieve clock rate for clock with id %d\n", clockId);
        return -1;
    }

    return mailbuffer[6];
}

unsigned int Mailbox_SetClockRate(unsigned int clockId, unsigned int rate)
{
    volatile unsigned int mailbuffer[256] __attribute__((aligned(16)));

    unsigned int bufSize = 1;
    mailbuffer[bufSize++] = 0;                 // 1. Request indicator
    mailbuffer[bufSize++] = MBT_SET_CLOCKRATE; // 2. Tag
    mailbuffer[bufSize++] = 0x8;               // 3. Value buffer size
    mailbuffer[bufSize++] = 0x8;               // 4. Value size
    mailbuffer[bufSize++] = clockId;           // 5. Id of clock to change (see mb_clock_id enum)
    mailbuffer[bufSize++] = rate;              // 6. New clock rate (in Hz)
    mailbuffer[bufSize++] = 0;                 // 7. End of message
    mailbuffer[0] = bufSize * 4;               // 0. Size of this buffer

	unsigned int bufferAddr = (unsigned int)&mailbuffer;
	bufferAddr -= KERNEL_VA_START;

	Mailbox_Write(8, bufferAddr);

    if (mailbuffer[1] != RESPONSE_SUCCESS)
    {
        printf("Failed to set clock rate for clock %d, Invalid mailbox response.\n", clockId);
        return -1;
    }

    if (mailbuffer[6] == 0)
    {
        printf("Failed to set clock rate for clock %d, Invalid clock id.\n", clockId);
        return -1;
    }

    return mailbuffer[6];
}

int Mailbox_SetDevicePowerState(unsigned int deviceId, unsigned int powerState)
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
	
	unsigned int bufferAddr = (unsigned int)&mailbuffer;
	bufferAddr -= KERNEL_VA_START;

	Mailbox_Write(8, bufferAddr);
	
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
		
	return 0;
}
