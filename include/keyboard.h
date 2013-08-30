#ifndef PIOS_KEYBOARD_C
#define PIOS_KEYBOARD_C

typedef enum 
{
	VK_A, VK_B, VK_C, VK_D, VK_E, VK_F, VK_G, VK_H, VK_I, VK_J,
	VK_K, VK_L, VK_M, VK_N, VK_O, VK_P, VK_Q, VK_R, VK_S, VK_T,
	VK_U, VK_V, VK_W, VK_X, VK_Y, VK_Z, VK_0, VK_1, VK_2, VK_3,
	VK_4, VK_5, VK_6, VK_7, VK_8, VK_9, VK_ENTER, VK_ESC, VK_BSP, VK_TAB,
	VK_SPACE, VK_MINUS, VK_EQUALS, VK_SBLEFT, VK_SBRIGHT, VK_BACKSLASH, VK_HASH, VK_SEMICOLON, VL_APOSTROPHE, 
	VK_GRAVE, VK_COMMA, VK_FULLSTOP, VK_FORWARDSLASH, VK_CAPS, VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, 
	VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12, VK_PRTSCN, VK_SCL, VK_INSERT, 
	VK_HOME, VK_PGUP, VK_DELETE, VK_END, VK_PGDN, VK_RIGHT, VK_LEFT, VK_DOWN, VK_UP, VK_NUMLOCK, VK_UNKNOWN,
	NUM_DIV, NUM_MUL, NUM_MIN, NUM_ADD, NUM_ENTER, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9, NUM0, NUM_DEL
} virtualkey;

unsigned int KeyboardInitialise(void);
void KeyboardUpdate(void);
unsigned short KeyboardGetChar(void);
bool EnsureKeyboard(void);
virtualkey ScanToVirtual(unsigned int scanCode);
unsigned char VirtualToAsci(virtualkey vk);

#endif
