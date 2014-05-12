#define SCREEN_WIDTH 640 // 1366
#define SCREEN_HEIGHT 480 //768

int Fb_Initialize();
void Fb_DrawPixel(unsigned int x, unsigned int y, unsigned short int color);
void Fb_DrawCharacterAt(unsigned int c, unsigned int x, unsigned int y);
void Fb_DrawColoredCharacterAt(unsigned int ch, unsigned int x, unsigned int y, unsigned short color);
