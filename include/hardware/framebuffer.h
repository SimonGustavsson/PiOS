#define FB_DEBUG
#define CHAR_HEIGHT 10
#define CHAR_WIDTH 6
#define FB_BPP 16
#define PREFERRED_WIDTH 1366
#define PREFERRED_HEIGHT 768

#include "types/types.h"

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
