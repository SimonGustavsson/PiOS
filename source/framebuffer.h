#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

int InitializeFramebuffer();
void DrawPixel(unsigned int x, unsigned int y, unsigned short int color);
void DrawCharacterAt(unsigned int c, unsigned int x, unsigned int y);