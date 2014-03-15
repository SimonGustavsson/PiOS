#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768

int InitializeFramebuffer();
void DrawPixel(unsigned int x, unsigned int y, unsigned short int color);
void DrawCharacterAt(unsigned int c, unsigned int x, unsigned int y);
void DrawColoredCharacterAt(unsigned int ch, unsigned int x, unsigned int y, unsigned short color);
