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
		if(strcasecmp(name, gCommands[i].name) == 0)
			return &gCommands[i];

	return 0;
}

unsigned int Command_About_Execute(char** args, unsigned int argCount)
{
	printf("PiOS - by Simon Gustavsson 2013\n");
	
	return 1;
}

unsigned int Command_Help_Execute(char** args, unsigned int argCount)
{
	if(argCount > 1)
	{
		// Print command specific help
		TerminalCommand* cmd = TerminalGetCommand(args[1]);
		
		if(cmd == 0)
		{
			printf("help: Unknown command '%s'.", args[1]);
			return -2;
		}
		
		printf("%s:\n%s", cmd->name, cmd->description);
		
		return 1;
	}
	
	printf("For extended help on a specific command, type 'help command-name'.\n");
	
	unsigned int i;
	for(i = 0; i < MAX_COMMAND_COUNT; i++)
	{
		// Array might contain empty slots
		if(gCommands[i].name != 0)
			printf("%s\t\t%s\n", gCommands[i].name, gCommands[i].description);
	}
	
	return 1;
}

unsigned int Command_Test_Execute(char** args, unsigned int argCount)
{
	printf("::Test Command:: Arguments:\n");
	
	unsigned int i;
	for(i = 0; i < argCount; i++)
		printf("%d = %s\n", i, args[i]);
		
	return 1;
}

void TerminalInitCommands(void)
{
	gCommands[0].name = "about";
	gCommands[0].description = "Prints information about PiOS.";
	gCommands[0].execute = &Command_About_Execute;
		
	gCommands[1].name = "help";
	gCommands[1].description = "Prints a listing of commands.";
	gCommands[1].execute = &Command_Help_Execute;
	
	gCommands[2].name = "test";
	gCommands[2].description = "Command for testing new things.";
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
			
	char* str = --cmdPtr;
	
	if(*str == '\0')
		return 0; // Empty string
		
	char* arguments[MAX_COMMAND_ARGUMENT_COUNT];
	unsigned int argCount = 0;
	
	// Split the string by spaces
	bool buildingString = false;
	while((currentChar = *cmdPtr) != '\0')
	{
		if(currentChar == delimiter && buildingString)
		{
			*cmdPtr = '\0';
			arguments[argCount++] = str;
			buildingString = false;
		}
		else if(!buildingString && currentChar != delimiter)
		{
			buildingString = true;
			str = cmdPtr; // start building new arg
		}
		
		cmdPtr++;
	}
	
	if(*str != '\0')
		arguments[argCount] = str;
	
	TerminalCommand* command = TerminalGetCommand(arguments[0]);
	
	if(command == 0)
	{
		if(argCount > 0)
			printf("command with name %s not found\n", arguments[0]);
		else
			printf("I can't execute nothing, dummy!\n");
			
		return -1; // Unknown command
	}
	
	return command->execute(arguments, argCount + 1); // Index to count, add one
}
