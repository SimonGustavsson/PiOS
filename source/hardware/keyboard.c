//#include "types.h"
//#include "usbd/usbd.h"
//#include "device/hid/keyboard.h"
#include "terminal.h"
#include "util/stringutil.h"
#include "hardware/keyboard.h"

char* gKeynames[] =
{
	"a", // 0
	"b",
	"c",
	"d",
	"e",
	"f",
	"g",
	"h",
	"i",
	"j",
	"k", // 10
	"l",
	"m",
	"n",
	"o",
	"p",
	"q",
	"r",
	"s",
	"t",
	"u", // 20
	"v",
	"w",
	"x",
	"y",
	"z",
	"1", 
	"2", 
	"3", 
	"4", 
	"5", // 30 
	"6", 
	"7", 
	"8", 
	"9",
	"0", 
	"return",
	"esc",
	"bsp",
	"tab",
	"space", // 40
	"minus",
	"equals",
	"lsquarebracket",
	"rsquarebracket",
	"backslash", // NO
	"hash",
	"semicolon",
	"apostrophe",
	"grave",
	"comma", // 50
	"fullstop",
	"forwardflash",
	"caps-lock",
	"f1",
	"f2",
	"f3",
	"f4",
	"f5",
	"f6",
	"f7", // 60
	"f8",
	"f9",
	"f10",
	"f11",
	"f12",
	"prtscn",
	"scroll-lock",
	"break",
	"insert",
	"home", // 70
	"page-up",
	"delete",
	"end",
	"page-down",
	"right",  //5
	"left", // 6
	"down", // 7
	"up", // 8
	"num-lock",
	"numdiv", // 80
	"nummul",
	"numsub",
	"numadd",
	"numreturn",
	"num1",
	"num2",
	"num3",
	"num4",
	"num5",
	"num6", // 90
	"num7",
	"num8",
	"num9",
	"num0",
	"numdelete",
	"backslash",
	"rmenu"
}; 

const char gVirtualTable_shifted[] =
{
	0, 
	0, 
	0, 
	0, 
	'A', 
	'B', 
	'C', 
	'D', 
	'E', 
	'F', 
	'G',
	'H', 
	'I', 
	'J',
	'K',  // 10 
	'L', 
	'M',
	'N',	
	'O', 
	'P',
	'Q', 
	'R', 
	'S', 
	'T', 
	'U',  // 20
	'V', 
	'W', 
	'X', 
	'Y', 
	'Z',
	'!',  
	'"', 
	'*', 
	'$', // 30
	'%', 
	'^', 
	'&', 
	'*',
	'(',
	')',
	'\n', 
	/*esc*/0, 
	/*Bsp*/0, 
	'\t',
	' ', // 40
	'_', 
	'+', 
	'{',
	'}',
	'|',
	'~',  
	':', 
	'@', 
	'*', 
	'<', // 50
	'>', 
	'?', 
	0, 
	0,
	0,
	0, 
	0, 
	0, 
	0, 
	0, // 60
	0, 
	0, 
	0, 
	0, 
	0,
	0, 
	0, 
	0, 
	0, 
	0, // 70
	0, 
	0, 
	0, 
	0, 
	0,
	0,  
	0, 
	0, 
	0, 
	'/', // 80
	'*', 
	'-', 
	'+', 
	'\n',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6', // 90
	'7',
	'8',
	'9',
	'0',
	0, // Numpad delete
	'\\'
};

const char gVirtualTable[] = 
{
	0, 
	0, 
	0, 
	0, 
	'a', 
	'b', 
	'c', 
	'd', 
	'e', 
	'f', 
	'g',
	'h', 
	'i', 
	'j',
	'k',  // 10 
	'l', 
	'm',
	'n',	
	'o', 
	'p',
	'q', 
	'r', 
	's', 
	't', 
	'u',  // 20
	'v', 
	'w', 
	'x', 
	'y', 
	'z',
	'1',  
	'2', 
	'3', 
	'4', // 30
	'5', 
	'6', 
	'7', 
	'8',
	'9',
	'0',
	'\n', 
	/*esc*/0, 
	/*Bsp*/0, 
	'\t',
	' ', // 40
	'-', 
	'=', 
	'[',
	']',
	'\\',
	'#',  
	';', 
	'\'', 
	'`', 
	',', // 50
	'.', 
	'/', 
	0, 
	0,
	0,
	0, 
	0, 
	0, 
	0, 
	0, // 60
	0, 
	0, 
	0, 
	0, 
	0,
	0, 
	0, 
	0, 
	0, 
	0, // 70
	0, 
	0, 
	0, 
	0, 
	0,
	0,  
	0, 
	0, 
	0, 
	'/', // 80
	'*', 
	'-', 
	'+', 
	'\n',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6', // 90
	'7',
	'8',
	'9',
	'0',
	0, // Numpad delete
	'\\'
};

bool gHaveKeyboard;
unsigned int gKeyboardAddr;
unsigned short gLastKeystate[6];
//struct KeyboardModifiers gLastModifiers;

bool KeyboardCtrlDown(void)
{
	return -1;//gLastModifiers.LeftControl || gLastModifiers.RightControl;
}

bool KeyboardShiftDown(void)
{
	return -1;//gLastModifiers.LeftShift || gLastModifiers.RightShift;
}

virtualkey ScanToVirtual(unsigned int scanCode)
{
	if(scanCode < 0) return VK_INVALID;
	
	return (virtualkey)(scanCode - 4);
}

unsigned char VirtualToAsci(virtualkey vk, bool shifted)
{
	if(vk < 0 || vk > (sizeof(gVirtualTable) / sizeof(gVirtualTable[0]))) return -1;
	
	if(shifted)
		return gVirtualTable_shifted[vk + 4];
	else
		return gVirtualTable[vk + 4];
}

char* GetKeyName(char* buf, unsigned int bufLen, virtualkey vk)
{
	char* keyName = gKeynames[vk];
	
	if(strlen(keyName) < bufLen)
		strcpy(keyName, buf);
		
	return buf;
}

// Retrieves the address of the first attached keyboard
bool EnsureKeyboard(void)
{	
	// KeyboardUpdate() modifies this
	if(gHaveKeyboard) 
		return true;
	
	//UsbCheckForChange();
	
	if(1)//KeyboardCount() == 0)
	{
		gHaveKeyboard = false;
		return false;
	}
	
	gKeyboardAddr = 0;//KeyboardGetAddress(0);
	gHaveKeyboard = true;
	
	return true;
}

bool KeyWasDown(unsigned short scanCode)
{
	unsigned int i;
	for(i = 0; i < 6; i++)
		if(gLastKeystate[i] == scanCode) return true;
		
	return false;
}

// Gets the scanCode of the first currently pressed key (0 if none)
unsigned short KeyboardGetChar(void)
{
	unsigned int i;
	for(i = 0; i < 6; i++)
	{
		unsigned short key = 0;//KeyboardGetKeyDown(gKeyboardAddr, i);
		
		if(key == 0) return 0;
		if(KeyWasDown(key)) continue;
		if(key >= 104) continue;
		
		return key;
	}
	
	return 0;
}

unsigned int KeyboardInitialise(void)
{
	gHaveKeyboard = false;
	gKeyboardAddr = 0;

	unsigned int i;
	for(i = 0; i < 6; i++)
		gLastKeystate[i] = 0;
		
	return 0;
}

void KeyboardUpdate(void)
{
	if(!EnsureKeyboard())
	{
		//print("Failed to update keyboard, could not obtain device.", strlen("Failed to update keyboard, could not obtain device."));
		return;
	}
	
	unsigned int i;
	for(i = 0; i < 6; i++)
		gLastKeystate[i] = 0;//KeyboardGetKeyDown(gKeyboardAddr, i);
		
	//gHaveKeyboard = KeyboardPoll(gKeyboardAddr) == 0;
	//gLastModifiers = KeyboardGetModifiers(gKeyboardAddr);
}
