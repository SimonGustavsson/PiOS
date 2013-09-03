#define MAX_COMMAND_COUNT 20
#define MAX_COMMAND_ARGUMENT_COUNT 10
#include "terminal_commands.h"
#include "terminal.h"
#include "stringutil.h"
#include "types.h"

TerminalCommand gCommands[MAX_COMMAND_COUNT];

TerminalCommand *TerminalGetCommand(char* name)
{
	unsigned int i;
	for(i = 0; i < MAX_COMMAND_COUNT; i++)
		if(strcasecmp(name, gCommands[i].name))
			return &gCommands[i];

	return 0;
}

unsigned int Command_About_Execute(char** args, unsigned int argCount)
{
	printf("PiOS - by Simon Gustavsson 2013");
	
	return 1;
}

unsigned int Command_Help_Execute(char** args, unsigned int argCount)
{
	printf("This is where the help goes.");
	
	return 1;
}

unsigned int Command_Test_Execute(char** args, unsigned int argCount)
{
	printf("Executing help command\n");
	printf("Arguments passed in:\n");
	
	unsigned int i;
	for(i = 0; i < argCount; i++)
		printf("%d %s", i, args[i]);
		
	return 1;
}

void TerminalInitCommands(void)
{
	gCommands[0].name = "about";
	gCommands[0].execute = &Command_About_Execute;
		
	gCommands[1].name = "help";
	gCommands[1].execute = &Command_Help_Execute;
	
	gCommands[2].name = "test";
	gCommands[2].execute= &Command_Test_Execute;
}

// 0: OK, -1: Unknown command
unsigned int TerminalExecuteCommand(char* cmd)
{
	char delimiter = ' ';
	char currentChar;
	char* cmdPtr = cmd;
	
	// Find first non-space character
	while((currentChar = *cmdPtr++))
		if(currentChar != delimiter)
			break;
			
	if(currentChar == '\0')
		return 0; // Empty string
		
	char* str = cmdPtr; // holds the current argument
	char* arguments[MAX_COMMAND_ARGUMENT_COUNT];
	char** argPtr = arguments;
	unsigned int argCount = 0;
	
	// Split the string by spaces
	bool buildingString = false;
	while((currentChar = *cmdPtr++) != '\0')
	{
		if(currentChar == delimiter && buildingString)
		{
			cmdPtr[-1] = '\0';
			*argPtr = str;
			argPtr++;
			buildingString = false;
			argCount++;
		}
		else
			buildingString = true;
	}
	
	TerminalCommand* command = TerminalGetCommand(arguments[0]);
	if(command == 0)
		return -1; // Unknown command
	
	return command->execute(arguments, argCount);
}
