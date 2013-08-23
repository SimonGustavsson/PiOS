#include "types.h"
#include "usbd/usbd.h" // For UsbCheckForChange()
#include "device/hid/keyboard.h"
#include "terminal.h"
#include "stringutil.h"

unsigned int gKeyboardAddr;
bool gHaveKeyboard;
unsigned short gLastKeyState[6];

void stdio_init(void)
{
	gHaveKeyboard = false;
	gKeyboardAddr = 0;

	unsigned int i;
	for(i = 0; i < 6; i++)
		gLastKeyState[i] = 0;
}

bool EnsureKeyboard()
{
	if(gHaveKeyboard == 1) return true; // Already got a keyboard

	UsbCheckForChange();
	
	if(KeyboardCount() == 0)
	{
		gHaveKeyboard = 0;
		return false;
	}

	gKeyboardAddr = KeyboardGetAddress(0); // Hard coded always use first keyboard
	gHaveKeyboard = 1;

	return true;
}

bool KeyWasDown(unsigned short scanCode)
{
	unsigned int i;
	for(i = 0; i < 6; i++)
		if(gLastKeyState[i] == scanCode) return true;

	return false;
}

char KeyboardGetChar(void)
{
	// Gets the next char that is pressed
	unsigned int i;
	for(i = 0; i < 6; i++)
	{
		unsigned short key = KeyboardGetKeyDown(gKeyboardAddr, i);
		
		if(key == 0)
			return 0; // we're done
		
		if(KeyWasDown(key))
			continue; // Not a new press
		
		if(key >= 104)
			continue; // Out of range? Or something

		// Get modifiers here, we've got a new press!

		// Translate scancode to asci character
		return (char)key;
	}

	return 0; // Nothing pressed :-(
}

void KeyboardUpdate(void)
{
	if(!EnsureKeyboard())
	{
		print("Failed to update keyboard\n", strlen("Failed to update keyboard\n"));
		return;
	}

	unsigned int i;
	for(i = 0; i < 6; i++)
		gLastKeyState[i] = KeyboardGetKeyDown(gKeyboardAddr, i);

	gHaveKeyboard = KeyboardPoll(gKeyboardAddr) == 0;
}

void cin(char* buf, unsigned int bufLen)
{
	unsigned int readChars = 0;
	char temp[bufLen];
	
	if(!EnsureKeyboard()) 
		return; // We have no keyboard
	
	do
	{
		char k = KeyboardGetChar();
		if(k == 0)
			continue;
		
		if(k == '\n')
			break;

		temp[readChars] = k;
		readChars++;

	}while(readChars < bufLen); // Just keep reading (Return cancels this early)

	if(readChars < bufLen - 1)
		temp[readChars + 1] = '\0';
	else
		temp[bufLen - 1] = '\0';

	// Copy the read stuff into the buffer
	strcpy(temp, buf);
}