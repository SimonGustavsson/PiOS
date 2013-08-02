#include "mailbox.h"
#include "myfont.h"

volatile unsigned int gPitch = 0;
volatile unsigned int gFbAddr;
volatile unsigned int gScreenWidth, gScreenHeight;
static int caretX = 25;
static int caretY = 25;
#define CHAR_HEIGHT 10
#define CHAR_WIDTH 6
#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768

void DrawPixel(unsigned int x, unsigned int y, unsigned short int color)
{
	unsigned short int* ptr;
	unsigned int offset;
	
	offset = (y * gPitch) + (x * 2);
	ptr = (unsigned short int*)(gFbAddr + offset);
	*ptr = color;
}

void Write(char* text)
{
	unsigned int row;
	int col;
	unsigned char ch;
	
	while((ch = (unsigned char)*text))
	{
		text++; // Advance to the next character
		
		// Make sure the char is within our table  0 - 95
		if(ch < 32)
			ch = 0;
		else
		{
			if(ch > 127)
				ch = 0;
			else
				ch -= 32;
		}
		
		for(row = 0; row < CHAR_HEIGHT; row++)
		{
			unsigned int i = 0;
			for(col = CHAR_HEIGHT - 2; col >= 0 ; col--)
			{
				if(row < (CHAR_HEIGHT - 1) && (teletext[ch][row] & (1 << col)))
				{
					DrawPixel(caretX + i, caretY + row, 0xFFFF);
				}
				else
				{
					DrawPixel(caretX + i, caretY + row, 0x0000);
				}
				i++;
			}
		}
		
		caretX += CHAR_WIDTH + 4; // pixel H space
		
		// TODO: CHECK FOR NEW LINE ETC
	}	
}
// 0: Success. 1: Invalid response to property request, 2: Invalid screen size returned
int GetScreenSizeFromTags()
{
	volatile unsigned int mailbuffer[256] __attribute__ ((aligned (16)));
	unsigned int mailbufferAddr = (unsigned int)mailbuffer;
	
	// Step1: get the display size
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
		
	gScreenWidth = mailbuffer[5];
	gScreenHeight = mailbuffer[6];
		
	if(gScreenWidth == 0 || gScreenHeight == 0)
		return 2;
		
	return 0;
}

// 0: Success, 1: Invalid response to Setup screen request, 2: Framebuffer setup failed, Invalid tags, 3: Invalid tag response, 4: Invalid tag data
int SetupScreen()
{
	volatile unsigned int mailbuffer[256] __attribute__ ((aligned (16)));
	unsigned int mailbufferAddr = (unsigned int)mailbuffer;
	
	mailbuffer[0] = 8 * 4; // NOT SURE IF WE NEED THIS
	
	// Set up the screen
	unsigned int c = 1;
	mailbuffer[c++] = 0; // This is a request
	mailbuffer[c++] = 0x00048003;	// Tag id (set physical size)
	mailbuffer[c++] = 8;		// Value buffer size (bytes)
	mailbuffer[c++] = 8;		// Req. + value length (bytes)
	mailbuffer[c++] = SCREEN_WIDTH;	// Horizontal resolution
	mailbuffer[c++] = SCREEN_HEIGHT;		// Vertical resolution

	mailbuffer[c++] = 0x00048004;	// Tag id (set virtual size)
	mailbuffer[c++] = 8;		// Value buffer size (bytes)
	mailbuffer[c++] = 8;		// Req. + value length (bytes)
	mailbuffer[c++] = SCREEN_WIDTH;		// Horizontal resolution
	mailbuffer[c++] = SCREEN_HEIGHT;		// Vertical resolution

	mailbuffer[c++] = 0x00048005;	// Tag id (set depth)
	mailbuffer[c++] = 4;		// Value buffer size (bytes)
	mailbuffer[c++] = 4;		// Req. + value length (bytes)
	mailbuffer[c++] = 16;		// 16 bpp

	mailbuffer[c++] = 0x00040001;	// Tag id (allocate framebuffer)
	mailbuffer[c++] = 8;		// Value buffer size (bytes)
	mailbuffer[c++] = 4;		// Req. + value length (bytes)
	mailbuffer[c++] = 16;		// Alignment = 16
	mailbuffer[c++] = 0;		// Space for response

	mailbuffer[c++] = 0;		// Terminating tag

	mailbuffer[0] = c*4;		// Buffer size

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
	
	if(gFbAddr == 0 || screenSize == 0)
		return 4;
		
	return 0;
}

// 0: Success, 1: Invalid pitch response, 2: Invalid pitch response
int GetPitch()
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
				
	//////// --------------- ALL GOOD ABOVE :))))))) ------------------------ //////////
	
	unsigned int pitch = mailbuffer[5];
	if(pitch == 0)
		return 2; // Invalid pitch response
		
	gPitch = pitch;
	
	return 0;
}

int InitializeFramebuffer()
{	
	unsigned int result = 0;
	
	if((result = GetScreenSizeFromTags()) > 0)
	{
		return result;
	}
	
	if((result = SetupScreen()) > 0)
	{
		return result;
	}
	
	if((result = GetPitch()) > 0)
	{
		return result;
	}
	
	return result;
}