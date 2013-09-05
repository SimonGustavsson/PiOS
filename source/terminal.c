#define CHAR_HEIGHT 10
#define CHAR_WIDTH 6
#define CHAR_VSPACING 4
#define CHAR_HSPACING 4
#define TERMINAL_WIDTH (SCREEN_WIDTH / (CHAR_WIDTH + CHAR_HSPACING)) - 1 // 191 @ 1920
#define TERMINAL_HEIGHT (SCREEN_HEIGHT / (CHAR_HEIGHT + CHAR_VSPACING)) - 1 // 76 @ 1080
#define BUFFER_HEIGHT TERMINAL_HEIGHT
#define BUFFER_WIDTH TERMINAL_WIDTH
#define INPUT_BUFFER_SIZE 256

#include "framebuffer.h"
#include "keyboard.h"
#include "stringutil.h"
#include "terminal_commands.h"

// Forward declare
void PresentBufferToScreen(void);

// TODO 0: Make buffer larger than display to allow for some history to get saved
// TODO 1: Change to int buffers and embed colors in value
char gBuffer[BUFFER_HEIGHT][BUFFER_WIDTH];
char gTerminal[TERMINAL_HEIGHT][TERMINAL_WIDTH];
int gBufferCaretRow; 		// The current row of the caret - where  text will be written to
int gBufferCaretCol;		// The current column of the caret - where text will be written to
int gFirstVisibleBufferRow; // The row in the first buffer that is currently the first row on screen

char gInputBuffer[INPUT_BUFFER_SIZE];
unsigned int gInputBufferIndex;

char gPrompt[TERMINAL_PROMPT_MAX_LENGTH] = "PiOS->";
unsigned int gPromptLength = 6;

void terminal_printWelcome(void)
{
	printf("##############################################################################\n");
	printf("############################# Welcome to PiOS 0.1 ############################\n");
	printf("###################### Available commands are: 'about'      ##################\n");
	printf("##################                                              ##############\n");
	printf("######################                                      ##################\n");
	printf("############################# By Simon Gustavsson ############################\n");
	printf("##############################################################################\n");
}

void terminal_printPrompt(void)
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
		
	PresentBufferToScreen();
}

void ExecuteCommand(char* cmd, unsigned int cmdLen)
{
	TerminalExecuteCommand(cmd);
	
	// TODO: Do something with the result of the command?
	
	terminal_printPrompt();
	
	// (Caller will present buffer)
}

void terminal_update(void)
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
	for(row = 0; row < TERMINAL_HEIGHT; row++)
	{
		for(col = 0; col < TERMINAL_WIDTH; col++)
		{
			if(gTerminal[row][col] != gBuffer[row][col])
			{
				gTerminal[row][col] = gBuffer[row][col];
				DrawCharacterAt(gTerminal[row][col], col * (CHAR_WIDTH + CHAR_HSPACING), row * (CHAR_HEIGHT + CHAR_VSPACING));
			}
		}
	}
}

void terminal_clear(void)
{
	gFirstVisibleBufferRow = 0;
	gBufferCaretRow = 0;
	gBufferCaretCol = 0;
	
	unsigned int row;
	for(row = 0; row < BUFFER_HEIGHT; row++)
	{
		unsigned int col;
		for(col = 0; col < BUFFER_WIDTH; col++)
		{
			gBuffer[row][col] = ' ';
		}
	}
	
	// Sync to display
	PresentBufferToScreen();
}

int terminal_init(void)
{
	if(InitializeFramebuffer() != 0)
		return -1;
	
	unsigned int i;
	for(i = 0; i < 256; i++)
		gInputBuffer[i] = 0;
		
	gInputBufferIndex = 0;

	// Initial setup of buffers etc
	terminal_clear();
	
	// Setup default built in commands
	TerminalInitCommands();
	
	return 0;
}

void terminal_back(void)
{
	if(gBufferCaretCol == 0)
	{
		gBufferCaretRow--;
		gBufferCaretCol = BUFFER_WIDTH - 1;
		
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

void print(char* string, unsigned int length)
{
	// 1. "Draw" everything to the buffer 
	unsigned int i;
	for(i = 0; i < length; i++)
	{	
		if(string[i] == '\n')
		{
			gBuffer[gBufferCaretRow][gBufferCaretCol] = ' ';
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
				if(gBufferCaretCol >= BUFFER_WIDTH - 1)
				{
					gBufferCaretCol = 0;
					gBufferCaretRow++;
				}
			}
		}
		else if(gBuffer[gBufferCaretRow][gBufferCaretCol] != string[i])
			gBuffer[gBufferCaretRow][gBufferCaretCol] = string[i];
		
		if(gBufferCaretCol < BUFFER_WIDTH - 1)
		{
			gBufferCaretCol++;
		}
		else
		{
			// Reached the end of this row
			gBufferCaretRow++;
			gBufferCaretCol = 0;
		}
		
		if(gBufferCaretRow >= BUFFER_HEIGHT)
		{
			terminal_clear();
			gBufferCaretRow = 0;
		}
	}
	
	// 2. Flip buffer to screen
	PresentBufferToScreen();
}
