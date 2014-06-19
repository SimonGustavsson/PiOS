#define CHAR_HEIGHT 10
#define CHAR_WIDTH 6
#define CHAR_VSPACING 4
#define CHAR_HSPACING 4
#define INPUT_BUFFER_SIZE 256

#include "hardware/framebuffer.h"
#include "hardware/keyboard.h"
#include "types/string.h"
#include "memory.h"
#include "terminalCommands.h"
#include "hardware/uart.h"
#include "hardware/timer.h"

size gBufferSize;
size gTerminalSize;

// Forward declare
void PresentBufferToScreen(void);

// TODO 0: Make buffer larger than display to allow for some history to get saved
// TODO 1: Change to int buffers and embed colors in value
char gBuffer[135][79];// [BUFFER_HEIGHT][BUFFER_WIDTH];
char gTerminal[135][79]; // [TERMINAL_HEIGHT][TERMINAL_WIDTH];
int gBufferCaretRow; 		// The current row of the caret - where  text will be written to
int gBufferCaretCol;		// The current column of the caret - where text will be written to
int gFirstVisibleBufferRow; // The row in the first buffer that is currently the first row on screen
int gTerminalInitialized;

char gInputBuffer[INPUT_BUFFER_SIZE];
unsigned int gInputBufferIndex;

char gPrompt[TERMINAL_PROMPT_MAX_LENGTH] = "PiOS->";
unsigned int gPromptLength = 6;
unsigned int gShowingTerminalPrompt;

void Terminal_PrintWelcome(void)
{
	printf("__/\\\\\\\\\\\\\\\\\\\\\\\\\\_______________/\\\\\\\\\\__________/\\\\\\\\\\\\\\\\\\\\\\___\n");
	printf(" _\\/\\\\\\/////////\\\\\\___________/\\\\\\///\\\\\\______/\\\\\\/////////\\\\\\_\n");
	printf("  _\\/\\\\\\_______\\/\\\\\\__/\\\\\\___/\\\\\\/__\\///\\\\\\___\\//\\\\\\______\\///__\n");
	printf("   _\\/\\\\\\\\\\\\\\\\\\\\\\\\\\/__\\///___/\\\\\\______\\//\\\\\\___\\////\\\\\\_________\n");
    printf("    _\\/\\\\\\/////////_____/\\\\\\_\\/\\\\\\_______\\/\\\\\\______\\////\\\\\\______\n");
	printf("     _\\/\\\\\\_____________\\/\\\\\\_\\//\\\\\\______/\\\\\\__________\\////\\\\\\___\n");
	printf("      _\\/\\\\\\_____________\\/\\\\\\__\\///\\\\\\__/\\\\\\_____/\\\\\\______\\//\\\\\\__\n");
	printf("       _\\/\\\\\\_____________\\/\\\\\\____\\///\\\\\\\\\\/_____\\///\\\\\\\\\\\\\\\\\\\\\\/___\n");
	printf("        _\\///______________\\///_______\\/////_________\\///////////_____\n");

	printf("Type 'help' for a listing of a few commands.\n");
}

void Terminal_PrintPrompt(void)
{
	// Make sure we print the prompt on a new row
	if(gBufferCaretCol > 0)
	{
		gBufferCaretRow++;
		gBufferCaretCol = 0;
	}

	unsigned int i;
	for(i = 0; i < gPromptLength; i++)
		gBuffer[gBufferCaretRow][gBufferCaretCol++] = gPrompt[i];
	
	// Print caret
	gBuffer[gBufferCaretRow][gBufferCaretCol] = (char)127;
		
	gShowingTerminalPrompt = 1;
		
	PresentBufferToScreen();
}

void ExecuteCommand(char* cmd, unsigned int cmdLen)
{
	TerminalCommands_Execute(cmd);
	
	// TODO: Do something with the result of the command?
	
	Terminal_PrintPrompt();
	
	// (Caller will present buffer)
}

void Terminal_Update(void)
{
	KeyboardUpdate();
		
	short scanCode = KeyboardGetChar();		
	
	// Nothing pressed
	if(scanCode == 0)
		return;
	
	virtualkey vk = ScanToVirtual(scanCode);

	if(vk == VK_ENTER)
	{
		// Remove cursor
		gBuffer[gBufferCaretRow][gBufferCaretCol] = 0;
		gInputBuffer[gInputBufferIndex] = '\0';
		
		// Give the command execution a fresh start on a new line
		gBufferCaretRow++;
		gBufferCaretCol = 0;
		
		// Execute the command
		ExecuteCommand(gInputBuffer, gInputBufferIndex);
		
		// Clear the input buffer
		unsigned int i;
		for(i = 0; i < INPUT_BUFFER_SIZE; i++)
			gInputBuffer[i] = 0;
			
		gInputBufferIndex = 0;
	}
	else if(vk == VK_BSP)
	{
		if(gInputBufferIndex == 0)
			return; // No characters left to delete!
			
		// Remove current cursor
		gInputBuffer[gInputBufferIndex] = 0; // Remove cursor
		//gBuffer[gBufferCaretRow][gInputBufferIndex] = 0;
		gBuffer[gBufferCaretRow][gBufferCaretCol] = 0;
		
		gBufferCaretCol--;
		gInputBufferIndex--;
		
		gInputBuffer[gInputBufferIndex] = (char)127;
		gBuffer[gBufferCaretRow][gBufferCaretCol] = gInputBuffer[gInputBufferIndex];
	}
	else
	{	
		char c;
		
		if((c = VirtualToAsci(vk, KeyboardShiftDown())) == 0 || gInputBufferIndex == INPUT_BUFFER_SIZE - 1)
			return;
		
		// Set input buffer (this will replace the cursor)
		gInputBuffer[gInputBufferIndex] = c;
		
		// Print the character (this will replace the cursor)
		gBuffer[gBufferCaretRow][gBufferCaretCol] = gInputBuffer[gInputBufferIndex];
		
		gBufferCaretCol++;
		gInputBufferIndex++;
		
		// Add cursor to input
		gInputBuffer[gInputBufferIndex] = (char)127;
		
		// Print cursor
		gBuffer[gBufferCaretRow][gBufferCaretCol] = gInputBuffer[gInputBufferIndex];
	}
	
	// Flip buffer to screen
	PresentBufferToScreen();
}

void PresentBufferToScreen(void)
{
	unsigned int row;
	unsigned int col;
    for (row = 0; row < gTerminalSize.height; row++)
	{
        for (col = 0; col < gTerminalSize.width; col++)
		{
			if(gTerminal[row][col] != gBuffer[row][col])
            {
				gTerminal[row][col] = gBuffer[row][col];
				Fb_DrawCharacterAt(gTerminal[row][col], col * (CHAR_WIDTH + CHAR_HSPACING), row * (CHAR_HEIGHT + CHAR_VSPACING));
			}
		}
	}
}

void Terminal_Clear(void)
{
	gFirstVisibleBufferRow = 0;
	gBufferCaretRow = 0;
	gBufferCaretCol = 0;
	
	unsigned int row;
    for (row = 0; row < gBufferSize.height; row++)
	{
		unsigned int col;
        for (col = 0; col < gBufferSize.width; col++)
		{
			gBuffer[row][col] = ' ';
		}
	}
	
	// Sync to display
	PresentBufferToScreen();
}

int Terminal_Initialize(void)
{
    // Just in case
    gTerminalInitialized = 0;
    gShowingTerminalPrompt = 0;
    gInputBufferIndex = 0;

#ifdef TERMINAL_DEBUG
    Uart_SendString("Init terminal.\n");
#endif

    if (Fb_Initialize() != 0)
        return -1;

    size screenSize = Fb_GetScreenSize();

    // TODO NOW:  Calculate gTerminalSize + gBufferSize
    gTerminalSize.width = 135;// (screenSize.width / (CHAR_WIDTH + CHAR_HSPACING)) - 1;
    gTerminalSize.height = 79;// (screenSize.height / CHAR_HEIGHT + CHAR_VSPACING) - 1;

    // For now we don't really bufer...
    gBufferSize.width = gTerminalSize.width;
    gBufferSize.height = gTerminalSize.height;

    //gBuffer = (char**)pcalloc(sizeof(char), gBufferSize.height * gBufferSize.width);
    //gTerminal = (char**)pcalloc(sizeof(char), gTerminalSize.height * gTerminalSize.width);
    	
    if (gBuffer == 0)
    {
        Uart_SendString("Failed to allocate terminal buffer\n");
        return -1;
    }
    
	unsigned int i;
	for(i = 0; i < 256; i++)
		gInputBuffer[i] = 0; // TODO: we don't have to do this, bss is initialized to 0?

	// Setup default built in commands
    TerminalCommands_Initialize();

    printf("Terminal size: %dx%d\n", gBufferSize.width, gBufferSize.height);
	
	gTerminalInitialized = 1;

	return 0;
}

int Terminal_GetIsInitialized(void)
{
	return gTerminalInitialized;
}

void Terminal_back(void)
{
	if(gBufferCaretCol == 0)
	{
		gBufferCaretRow--;
		gBufferCaretCol = gBufferSize.width - 1;
		
		// We have to go back up a row
		gBuffer[gBufferCaretRow][gBufferCaretCol] = ' ';
	}
	else
	{
		gBufferCaretCol--;
		gBuffer[gBufferCaretRow][gBufferCaretCol] = ' ';
	}
	
	PresentBufferToScreen();
}

void print_internal(char* string, unsigned int length, unsigned int important)
{
	if(gTerminalInitialized == 0)
	{
        Uart_SendString("[Uart]");
		unsigned int j;
		for(j = 0; j < length; j++)
			Uart_Send(string[j]);

		return;
	}

	// TODO: This should first construct the string, THEN Send it off to a print function

	// 0. If we're currently showing the prompt (and potentially user input)
	// we overwrite this, and rewrite it again after this print
	if(gShowingTerminalPrompt && important)
	{
		// Remote the prompt and input
		// TODO: This only expects prompt and input to be on one row, fix that...
		unsigned int i;
		unsigned int charsToRemote = INPUT_BUFFER_SIZE + gInputBufferIndex;
		for(i = 0; i < charsToRemote; i++)
			gBuffer[gBufferCaretRow][i] = ' ';
		
		gBufferCaretCol = 0;
	}
	else if(!important)
	{
		// If this isn't an important message, but we were showing the prompt
		// Just append the text, and flag that we are no longer showing the prompt
		gShowingTerminalPrompt = 0;
	}
	
	// 1. "Draw" everything to the buffer 
	unsigned int i;
	for(i = 0; i < length; i++)
	{	
		if(string[i] == '\n')
		{
			Uart_Send('\n');
            Uart_Send('\r');

			//gBuffer[gBufferCaretRow][gBufferCaretCol] = ' ';
			gBufferCaretRow++;
			gBufferCaretCol = 0;
			
			// Print cursor
			gBuffer[gBufferCaretRow][gBufferCaretCol] = (char)127;
			continue;
		}
		
		if(string[i] == '\t')
		{
			// Print tabs as 4 spaces
			unsigned int k;
			for(k = 0; k < 4; k++)
			{
				gBuffer[gBufferCaretRow][gBufferCaretCol++] = ' ';
				
				// Make sure we move to the next row if we reach the end of the current row
				if(gBufferCaretCol >= gBufferSize.width - 1)
				{
					gBufferCaretCol = 0;
					gBufferCaretRow++;
				}
			}
            Uart_SendString("    ");
		}
		else if (gBuffer[gBufferCaretRow][gBufferCaretCol] != string[i])
		{
            Uart_Send(string[i]);
			gBuffer[gBufferCaretRow][gBufferCaretCol] = string[i];
		}
		else
		{
            Uart_Send(string[i]);
		}
		
        if (gBufferCaretCol < gBufferSize.width - 1)
		{
			gBufferCaretCol++;
		}
		else
		{
			// Reached the end of this row
			gBufferCaretRow++;
			gBufferCaretCol = 0;
		}
		
        if (gBufferCaretRow >= gBufferSize.height)
		{
			Terminal_Clear();
			gBufferCaretRow = 0;
		}
	}

    Uart_Send(0); // String over
	
	// Write the prompt back again - but only if we wrote an important message.
	// as we want multiple print() calls to be able to write to the same row
	if(gShowingTerminalPrompt && important)
	{
		Terminal_PrintPrompt();
		
		// Add the user input + caret
		unsigned int j;
		for(j = 0; j < gInputBufferIndex; j++)
			gBuffer[gBufferCaretRow][gBufferCaretCol++] = gInputBuffer[j];
	}
	
	// 2. Flip buffer to screen
	PresentBufferToScreen();
}

void Terminal_PrintImportant(char* string, unsigned int length)
{
	print_internal(string, length, 1);
}

void Terminal_Print(char* string, unsigned int length)
{
	print_internal(string, length, 0);
}
