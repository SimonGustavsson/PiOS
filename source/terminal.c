#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768
#define CHAR_HEIGHT 10
#define CHAR_WIDTH 6

#define TERMINAL_WIDTH SCREEN_WIDTH / CHAR_WIDTH
#define TERMINAL_HEIGHT SCREEN_HEIGHT / CHAR_HEIGHT

#define BUFFER_HEIGHT 128
#define BUFFER_WIDTH TERMINAL_WIDTH
#define CHAR_VSPACING 4
#define CHAR_HSPACING 4

#include "framebuffer.h"

int gTerminal[TERMINAL_WIDTH][TERMINAL_HEIGHT];
int gBuffer[BUFFER_WIDTH][BUFFER_HEIGHT];

int gFirstVisibleBufferRow; // The row in the first buffer that is currently the first row on screen
int gBufferCaretRow; 		// The current row of the caret - where  text will be written to
int gBufferCaretCol;		// The current column of the caret - where text will be written to

int terminal_init(void)
{
	if(InitializeFramebuffer() != 0)
	{
		return -1;
	}

	gFirstVisibleBufferRow = 0;
	gBufferCaretRow = 0;
	gBufferCaretCol = 0;
	
	unsigned int i;
	unsigned int j;
	// Set every character in the terminal buffer to "delete" to start with
	for(i = 0; i < TERMINAL_WIDTH; i++)
	{
		for(j = 0; j < TERMINAL_WIDTH; i++)
		{
			gTerminal[i][j] = (char)0x7f;
		}
	}

	for(i = 0; i < BUFFER_WIDTH; i++)
	{
		for(j = 0; j < BUFFER_HEIGHT; i++)
		{
			gTerminal[i][j] = (char)0x7f;
		}
	}	
	
	return(0);
}

// TODO: Add color overload?
void print(char* string, unsigned int length)
{
	unsigned int i;
	for(i = 0; i < length; i++)
	{
			// 1. Set character in buffer
		if(gBuffer[gBufferCaretRow][gBufferCaretCol] != string[i])
		{
			gBuffer[gBufferCaretRow][gBufferCaretCol] = string[i];
		}
		
		// 2. If this character is already displayed in the correct position on screen, we don't need to do anything
		if(gTerminal[gBufferCaretRow - gFirstVisibleBufferRow][gBufferCaretCol] == gBuffer[gBufferCaretRow][gBufferCaretCol])
		{
			// Already drawing this char at this location
			// Performance boost skipping this!
			gBufferCaretCol++;
			continue;
		}
		
		// 3. Update terminal to include new character
		gTerminal[gBufferCaretRow - gFirstVisibleBufferRow][gBufferCaretCol] = gBuffer[gBufferCaretRow][gBufferCaretCol];
		
		// 3. We've written something new to the terminal - display it on screen
		// TODO: Might be better to postpone this till after the string has been processed
		// 		 and do a full comparison on the buffer vs terminal?
		DrawCharacterAt(string[i], gBufferCaretCol * (CHAR_HSPACING + CHAR_WIDTH), gBufferCaretCol * (CHAR_HSPACING + CHAR_WIDTH));
		
		if(gBufferCaretCol < BUFFER_WIDTH)
		{
			gBufferCaretCol++;
		}
		else
		{
			// We've reached the end of this row
			gBufferCaretRow++;
			gBufferCaretCol = 0;
		}
	}
}
