#define FB_DEBUG
#define CHAR_HEIGHT 10
#define CHAR_WIDTH 6
#define FB_BPP 16
#define PREFERRED_WIDTH 640
#define PREFERRED_HEIGHT 480

#include "types/types.h"

// The raspberry pi framebuffer, this maps directly onto the mailbox buffer
// to send to retrieve the framebuffer
typedef struct {
	unsigned int width;    // Requested screen width
	unsigned int height;   // Requested screen height
	unsigned int v_width;  // Requested virtual screen width
	unsigned int v_height; // Requested virtual screen height
	unsigned int pitch;    // Initialize to 0, VC fills out with value
	unsigned int depth;    // BPP - Initialize to 0, VC fills out with value
	unsigned int offset_x; // X offset of virtual framebuffer
	unsigned int offset_y; // Y offset of virtual framebuffer
	unsigned int address;  // Initialize to 0, VC fills out with value
	unsigned int size;     // Initialize to 0, VC fills out with value
} rpi_fb;

int Fb_Initialize();
void Fb_DrawPixel(unsigned int x, unsigned int y, unsigned short int color);
void Fb_DrawCharacterAt(unsigned int c, unsigned int x, unsigned int y);
void Fb_DrawColoredCharacterAt(unsigned int ch, unsigned int x, unsigned int y, unsigned short color);
unsigned int Fb_GetPhyAddr(void);

// Gets the screen size in pixels
size Fb_GetScreenSize(void);

// Gets the size of the framebuffer in memory (in bytes)
unsigned int Fb_GetSize(void);

void Fb_Clear(void);
