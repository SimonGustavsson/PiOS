#include "types.h"
#include "usbd/usbd.h"
#include "device/hid/keyboard.h"
#include "terminal.h"
#include "stringutil.h"

unsigned int gKeyboardAddr;
bool gHaveKeyboard;
unsigned short gLastKeystate[6];

char KeyboardScanToChar(unsigned short scanCode)
{
	// a-z
	if(scanCode >= 4 && scanCode <= 97)
		return 97 + (scanCode - 4);
	
	// 0-9
	if(scanCode >= 30 && scanCode <= 39)
		return 48 + (scanCode - 30);
		
	switch(scanCode)
	{
		case 40: return '\n';
		
		case 44: return ' ';
		case 45: return '-';
		case 46: return '+';
		case 47: return '[';
		case 48: return ']';
		
		case 50: return '#';
		case 51: return ';';
		case 52: return '\'';
		case 53: return '`';
		case 54: return ',';
		case 55: return '.';
		case 56: return '/';
		default: return 'z';
	}
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
