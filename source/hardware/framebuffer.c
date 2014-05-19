#define CHAR_HEIGHT 10
#define CHAR_WIDTH 6
#define FB_BPP 16
#define PREFERRED_WIDTH 1366
#define PREFERRED_HEIGHT 768

#include "hardware/framebuffer.h"
#include "hardware/mailbox.h"
#include "myfont.h"
#include "types/string.h"
#include "types/types.h"

unsigned int gPitch;
unsigned int gFbAddr;
static size gScreenSize;

unsigned int Fb_GetSize(void)
{
    return gScreenSize.width * gScreenSize.height * FB_BPP;
}

unsigned int Fb_GetAddress(void)
{
    return gFbAddr;
}

size Fb_GetScreenSize(void)
{
    return gScreenSize;
}

void Fb_DrawPixel(unsigned int x, unsigned int y, unsigned short int color)
{
	unsigned short int* ptr;
	unsigned int offset;
	
	offset = (y * gPitch) + (x * 2);
	ptr = (unsigned short int*)(gFbAddr + offset);
	*ptr = color;
}

void Fb_DrawCharacterAt(unsigned int ch, unsigned int x, unsigned int y)
{
	Fb_DrawColoredCharacterAt(ch, x, y, 0xFFFF);
}

void Fb_DrawColoredCharacterAt(unsigned int ch, unsigned int x, unsigned int y, unsigned short color)
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
                Fb_DrawPixel(x + i, y + row, 0x1212); // Same as background
			}
			i++;
		}
	}
}

// 0: Success. 1: Invalid response to property request, 2: Invalid screen size returned
static int GetScreenSizeFromTags()
{
	volatile unsigned int mailbuffer[256] __attribute__ ((aligned (16)));
	unsigned int mailbufferAddr = (unsigned int)mailbuffer;
	
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
		
    gScreenSize.width = mailbuffer[5];
    gScreenSize.height = mailbuffer[6];
		
    if (gScreenSize.width == 0 || gScreenSize.height == 0)
		return 2;

#ifdef FB_DEBUG
    printf("Framebuffer: Got screen size: %dx%d\n", gScreenSize.width, gScreenSize.height);
#endif

	return 0;
}

// 0: Success, 1: Invalid response to Setup screen request, 2: Framebuffer setup failed, Invalid tags, 3: Invalid tag response, 4: Invalid tag data
static int SetupScreen()
{
	volatile unsigned int mailbuffer[256] __attribute__ ((aligned (16)));
	unsigned int mailbufferAddr = (unsigned int)mailbuffer;
	
	mailbuffer[0] = 8 * 4; // NOT SURE IF WE NEED THIS
	
	unsigned int c = 1;
	mailbuffer[c++] = 0;			 // This is a request
	mailbuffer[c++] = 0x00048003;	 // Tag id (set physical size)
	mailbuffer[c++] = 8;			 // Value buffer size (bytes)
	mailbuffer[c++] = 8;			 // Req. + value length (bytes)
	mailbuffer[c++] = PREFERRED_WIDTH;           // Horizontal resolution
	mailbuffer[c++] = PREFERRED_HEIGHT;           // Vertical resolution

	mailbuffer[c++] = 0x00048004;	   // Tag id (set virtual size)
	mailbuffer[c++] = 8;			   // Value buffer size (bytes)
	mailbuffer[c++] = 8;			   // Req. + value length (bytes)
    mailbuffer[c++] = PREFERRED_WIDTH; // Horizontal resolution
    mailbuffer[c++] = PREFERRED_HEIGHT;// Vertical resolution

	mailbuffer[c++] = 0x00048005;	   // Tag id (set depth)
	mailbuffer[c++] = 4;		       // Value buffer size (bytes)
	mailbuffer[c++] = 4;			   // Req. + value length (bytes)
    mailbuffer[c++] = FB_BPP;	       // 16 bpp

	mailbuffer[c++] = 0x00040001;	   // Tag id (allocate framebuffer)
	mailbuffer[c++] = 8;			   // Value buffer size (bytes)
	mailbuffer[c++] = 4;			   // Req. + value length (bytes)
    mailbuffer[c++] = FB_BPP;          // Alignment = 16
	mailbuffer[c++] = 0;			   // Space for response

	mailbuffer[c++] = 0;			   // Terminating tag

	mailbuffer[0] = c*4;			   // Buffer size

	Mailbox_Write(8, mailbufferAddr);
	
	Mailbox_Read(8);
	
	if(mailbuffer[1] != 0x80000000)
		return 1;
		
	unsigned int temp;
	unsigned int count = 2; // Read the first tag
	while((temp = mailbuffer[count]))
	{
		if(temp == 0x40001)
			break;
			
		count += 3 + (mailbuffer[count + 1] >> 2);		
		if(count > c)
			return 2; // Framebuffer setup failed, Invalid tags.
	}
	
	// 8 bytes, plus the MSB is set to indicate that this is a response
	if(mailbuffer[count + 2] != 0x80000008)
		return 3; // Invalid tag response
		
	gFbAddr = mailbuffer[count + 3];
	unsigned int screenSize = mailbuffer[count + 4];
	
	if(gFbAddr == 0)// || screenSize == 0)
		return 4;
		
#ifdef FB_DEBUG
    printf("Framebuffer intiialized, address: 0x%h\n", gFbAddr);
#endif

	return 0;
}

// 0: Success, 1: Invalid pitch response, 2: Invalid pitch response
static int GetPitch()
{
	volatile unsigned int mailbuffer[256] __attribute__ ((aligned (16)));
	unsigned int mailbufferAddr = (unsigned int)mailbuffer;
	
	// All super so far - Now time to get the pitch (bytes per line)
	mailbuffer[0] = 7 * 4; 		// Total Size
	mailbuffer[1] = 0; 			// This is a request
	mailbuffer[2] = 0x40008;	// Display size
	mailbuffer[3] = 4; 			// Buffer size
	mailbuffer[4] = 0;			// Request size
	mailbuffer[5] = 0; 			// REPONSE - Pitch
	mailbuffer[6] = 0;			// end tag
	
	Mailbox_Write(8, mailbufferAddr);
	
	Mailbox_Read(8);

	// 4 bytes, plus the MSB set to indicate a response
	if(mailbuffer[4] != 0x80000004)
		return 1; // Invalid pitch response
					
	unsigned int pitch = mailbuffer[5];
	if(pitch == 0)
		return 2; // Invalid pitch response
		
	gPitch = pitch;

#ifdef FB_DEBUG
    printf("Framebuffer: Got pitch: %d\n", gPitch);
#endif

	return 0;
}

int Fb_Initialize()
{	
	unsigned int result = 0;
	
	if((result = GetScreenSizeFromTags()) > 0)
	{
#ifdef FB_DEBUG
        printf("Framebuffer: Failed to get screen size from tags\n");
#endif
		return result;
	}
	
	if((result = SetupScreen()) > 0)
    {
#ifdef FB_DEBUG
        printf("Framebuffer: Failed to setup screen\n");
#endif
		return result;
	}
	
	if((result = GetPitch()) > 0)
    {
#ifdef FB_DEBUG
        printf("Framebuffer: Failed to get pitch\n");
#endif
		return result;
	}

    // Draw background color to make it visually obvious how large the drawing area is
    unsigned int i, j;
    for (i = 0; i < gScreenSize.width; i++)
    {
        for (j = 0; j < gScreenSize.height; j++)
            Fb_DrawPixel(i, j, 0x1212);
    }
	
	return result;
}
