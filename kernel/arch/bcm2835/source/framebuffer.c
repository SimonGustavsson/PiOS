#include "bcm2835_fb.h"
#include "bcm2835_mailbox.h"
#include "hardware/fb.h"
#include "memory_map.h"
#include "myfont.h"
#include "types/string.h"
#include "types/types.h"
#include "debugging.h"
#include "timer.h"

rpi_fb gFb __attribute__ ((aligned (16)));
size gScreenSize;
unsigned int gFbMaxAddr;

extern int gTerminalInitialized;

uint32_t Fb_GetSize(void)
{
    return gFb.size;
}

uint32_t Fb_GetPhyAddr(void)
{
    return gFb.address;
}

size Fb_GetScreenSize(void)
{
	size s;
	s.width = gFb.width;
	s.height = gFb.height;

    return s;
}

void Fb_DrawPixel(uint32_t x, uint32_t y, uint16_t color)
{
	// Offset into framebuffer
	unsigned int offset = (y * gFb.pitch) + (x * 2);

    unsigned short int* ptr = (unsigned short int*)((FRAMEBUFFER_VA_START) + offset);

    if((unsigned int)ptr >= gFbMaxAddr)
    {
    	gTerminalInitialized = 0; // Stop printing to Framebuffer

	   	printf("Framebuffer: Warning! Attempting to write outside buffer!\n");

	   	Debug_PrintCallstack(2);

	   	for(;;)
	   	{
	   		wait(2000);
	   	}

    }
    else
    {
		*ptr = color;
    }
}

void Fb_DrawCharacterAt(uint16_t ch, uint32_t x, uint32_t y)
{
	Fb_DrawColoredCharacterAt(ch, x, y, 0xFFFF);
}

void Fb_DrawColoredCharacterAt(uint16_t ch, uint32_t x, uint32_t y, uint16_t color)
{
	// Ensure valid char table lookup
	ch = ch < 32 ? 0 : ch > 127 ? 0 : ch - 32;
	
	int col;
	unsigned int row;
	for(row = 0; row < CHAR_HEIGHT; row++)
	{
		unsigned int i = 0;
		for(col = CHAR_HEIGHT - 2; col >= 0 ; col--)
		{
			if(row < (CHAR_HEIGHT - 1) && (gKernelFont[ch][row] & (1 << col)))
			{
                Fb_DrawPixel(x + i, y + row, color);
			}
			else
			{
                Fb_DrawPixel(x + i, y + row, 0x0000); // Same as background
			}
			i++;
		}
	}
}

// 0: Success. 1: Invalid response to property request, 2: Invalid screen size returned
static int32_t GetScreenSizeFromTags()
{
	volatile unsigned int mailbuffer[256] __attribute__ ((aligned (16)));
	unsigned int mailbufferAddr = (unsigned int)mailbuffer;
	mailbufferAddr -= KERNEL_VA_START;

	mailbuffer[0] = 8 * 4;		// Total size
	mailbuffer[1] = 0;			// Request
	mailbuffer[2] = 0x40003;	// Display size
	mailbuffer[3] = 8;			// Buffer size
	mailbuffer[4] = 0;			// Request size
	mailbuffer[5] = 0;			// Space for horizontal resolution
	mailbuffer[6] = 0;			// Space for vertical resolution
	mailbuffer[7] = 0;			// End tag
	
	Mailbox_Write(8, mailbufferAddr);
	
	Mailbox_Read(8);
	
	if(mailbuffer[1] != 0x80000000)
		return 1;

    if (mailbuffer[5] == 0 || mailbuffer[6] == 0)
		return 2;

	gScreenSize.width = mailbuffer[5];
	gScreenSize.height = mailbuffer[6];

#ifdef FB_DEBUG
    printf("Framebuffer: Got screen size: %dx%d\n", gScreenSize.width, gScreenSize.height);
#endif

	return 0;
}

void Fb_Clear(void)
{
    // Draw background color to make it visually obvious how large the drawing area is
    unsigned int i, j;
    for (i = 0; i < gFb.width; i++)
    {
        for (j = 0; j < gFb.height; j++)
            Fb_DrawPixel(i, j, 0x0000);
    }
}

int fb_allocateBuffer(void)
{
	int width = PREFERRED_WIDTH;
	int height = PREFERRED_HEIGHT;

	// Attempt to retrieve the physical resolution of the monitor
	if(GetScreenSizeFromTags() == 0)
	{
		width = gScreenSize.width;
		height = gScreenSize.height;

		printf("Framebuffer: Allocating buffer based on resolution retrieved from VC (%dx%d)\n", width, height);
	}
	else
	{
		printf("Framebuffer: WARNING - using default resolution (%dx%d).\n", width, height);
	}

	gFb.width = width;     // Requested width of the physical display
	gFb.height = height;   // Requested height of the physical display
	gFb.v_width = width;   // Requested width of the virtual framebuffer
	gFb.v_height = height; // Requested height of the virtual framebuffer
	gFb.pitch = 0;         // Pitch - Request: Set to 0, Response:  Number of bytes between each row of the frame buffer
	gFb.depth = FB_BPP;    // Requested depth (bits per pixel)
	gFb.offset_x = 0;      // Requested X offset of the virtual framebuffer
	gFb.offset_y = 0;      // Requested Y offset of the virtual framebuffer
	gFb.address = 0;       // Framebuffer address - Request: 0, Response: Address of buffer allocated by VC, or zero if request fails
	gFb.size = 0;          // Framebuffer size - Request: 0, Response: Size of buffer allocated by VC

	unsigned int fbStructAddr = (unsigned int)&gFb;

	// Mailbox requires the physical address
	// TODO: Add some sort of function to do this instead? Doing his in a lot of places now...
	fbStructAddr -= KERNEL_VA_START;

	Mailbox_Write(1, fbStructAddr);

	Mailbox_Read(1);

	if(gFb.address == 0)
		return 1; // Invalid FB address

	return 0;
}

int32_t Fb_Initialize()
{	
	unsigned int result = 0;
	gScreenSize.width = 0;
	gScreenSize.height = 0;

	
	// For some reason known to no one, allocating the buffer might fail
	// The first (few) time(s), so retry a couple of times (seems to succeed more often than not on the second)
	int tries = 1;
	do
	{
		result = fb_allocateBuffer();
	}while(tries++ < 5 && result != 0);

	if(result == 0)
	{
		printf("Framebuffer: Successfully retrieved buffer after %d tries.\n", tries);
		printf("Framebuffer: Allocated buffer at 0x%h, %d BPP, pitch: %d, %dx%d (virtual %dx%d)\n",
			gFb.address,
			gFb.depth,
			gFb.pitch,
			gFb.width,
			gFb.height,
			gFb.v_width,
			gFb.v_height);

		gFbMaxAddr = FRAMEBUFFER_VA_START + gFb.size;
		printf("Framebuffer: Framebuffer resides between 0x%h (phy: 0x%h) -> 0x%h (phy: 0x%h) (size: %d)\n", 
			FRAMEBUFFER_VA_START,
			gFb.address, 
			gFbMaxAddr,
			gFb.address + gFb.size, 
			gFb.size);
	}
	else
	{
		printf("Framebuffer: Failed to allocate framebuffer on VC\n");
	}

	return result;
}
