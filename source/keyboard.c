#include "types.h"
#include "usbd/usbd.h"
#include "device/hid/keyboard.h"
#include "terminal.h"
#include "stringutil.h"
#include "keyboard.h"

unsigned int gKeyboardAddr;
bool gHaveKeyboard;
unsigned short gLastKeystate[6];

char gVirtualTable[] = {
	0, 0, 0, 0, 'a', 'b', 'c', 'd', 'e', 'f', 
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',	'o', 'p',
	'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'1', '2', '3', '4', '5', '6', '7', '8',	'9', '0',
	'\n', /*esc*/0, /*Bsp*/0, '\t',	' ', '-', '=', '[', ']', '\\',
	'#', ';', '\'', '`', ',', '.', '/', 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, '\\',	
};

virtualkey ScanToVirtual(unsigned int scanCode)
{
	if(scanCode < 0) return VK_UNKNOWN;
	
	return (virtualkey)(scanCode - 4);
}

unsigned char VirtualToAsci(virtualkey vk)
{
	if(vk < 0 || vk > (sizeof(gVirtualTable) / sizeof(gVirtualTable[0]))) return -1;
	
	return gVirtualTable[vk + 4];
}

// Retrieves the address of the first attached keyboard
bool EnsureKeyboard(void)
{	
	// KeyboardUpdate() modifies this
	if(gHaveKeyboard) 
		return true;
	
	UsbCheckForChange();
	
	if(KeyboardCount() == 0)
	{
		gHaveKeyboard = false;
		return false;
	}
	
	gKeyboardAddr = KeyboardGetAddress(0);
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
		unsigned short key = KeyboardGetKeyDown(gKeyboardAddr, i);
		
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
		print("Failed to update keyboard, could not obtain device.", strlen("Failed to update keyboard, could not obtain device."));
		return;
	}
	
	unsigned int i;
	for(i = 0; i < 6; i++)
		gLastKeystate[i] = KeyboardGetKeyDown(gKeyboardAddr, i);
		
	gHaveKeyboard = KeyboardPoll(gKeyboardAddr) == 0;
}
