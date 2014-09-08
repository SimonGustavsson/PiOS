#define FB_DEBUG
#define CHAR_HEIGHT 10
#define CHAR_WIDTH 6
#define FB_BPP 16
#define PREFERRED_WIDTH 640
#define PREFERRED_HEIGHT 480

#include "stdint.h"
#include "types/types.h"

// Initializes the framebuffer
int32_t Fb_Initialize(void);

// Clears the framebuffer with all black
void Fb_Clear(void);

// Gets the physical address of the framebuffer
uint32_t Fb_GetPhyAddr(void);

// Gets size of Framebuffer in memory (in bytes)
uint32_t Fb_GetSize(void);

// Gets the screen size (in pixels)
size Fb_GetScreenSize(void);

void Fb_DrawPixel(uint32_t x, uint32_t y, uint16_t color);
void Fb_DrawCharacterAt(uint16_t c, uint32_t x, uint32_t y);
void Fb_DrawColoredCharacterAt(uint16_t c, uint32_t x, uint32_t y, uint16_t color);
