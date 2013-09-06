#define MAX_COMMAND_COUNT 20
#define MAX_COMMAND_ARGUMENT_COUNT 10
#include "terminal_commands.h"
#include "terminal.h"
#include "stringutil.h"
#include "types.h"

extern char gPrompt[TERMINAL_PROMPT_MAX_LENGTH];
TerminalCommand gCommands[MAX_COMMAND_COUNT];
unsigned int gCommandCount = 0;

void TerminalRegisterCommand(char* name, char* description, unsigned int (*execute)(char**, unsigned int))
{
	gCommands[gCommandCount].name = name;
	gCommands[gCommandCount].description = description;
	gCommands[gCommandCount].execute = execute;
	
	gCommandCount++;
}

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

unsigned int Command_Cls_Execute(char** args, unsigned int argCount)
{
	terminal_clear();
	
	return 1;
}

unsigned int Command_Echo_Execute(char** args, unsigned int argCount)
{
	unsigned int i;
	for(i = 1; i < argCount; i++)
	{
		printf("%s ", args[i]);
	}
	
	return 1;
}

unsigned int Command_Prompt_Execute(char** args, unsigned int argCount)
{
	if(argCount > 1)
	{
		unsigned int i;
		for(i = 0; i < TERMINAL_PROMPT_MAX_LENGTH - 4; i++)
		{
			if(args[1][i] == '\0')
				break;
			
			gPrompt[i] = args[1][i];
		}
		gPrompt[i] = '-';
		gPrompt[i + 1] = '>';
		gPrompt[i + 2] = '\0';
	}
	else
		printf("Not enough arguments, please specify a new prompt text.");
		
	return 1;
}

void TerminalInitCommands(void)
{
	TerminalRegisterCommand("about", "Prints information about PiOS.", &Command_About_Execute);
	TerminalRegisterCommand("help", "Prints a listing of commands.", &Command_Help_Execute);
	TerminalRegisterCommand("test", "Command for testing new things.", &Command_Test_Execute);
	TerminalRegisterCommand("cls", "Clears the terminal.", &Command_Cls_Execute);
	TerminalRegisterCommand("echo", "Prints the given string to the terminal.", &Command_Echo_Execute);
	TerminalRegisterCommand("prompt", "Changes the system prompt text.", &Command_Prompt_Execute);
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
		printf("command with name %s not found\n", arguments[0]);
		
		return -1; // Unknown command
	}
	
	return command->execute(arguments, argCount + 1); // Index to count, add one
}
