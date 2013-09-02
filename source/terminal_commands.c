#define MAX_COMMAND_COUNT 20
#include "terminal_commands.h"
#include "terminal.h"
#include "stringutil.h"

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
