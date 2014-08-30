#ifndef TERMINAL_COMMANDS
#define TERMINAL_COMMANDS

#define TERMINAL_PROMPT_MAX_LENGTH 10
typedef struct
{
	char* name;
	char* description;
	unsigned int (*execute)(char**, unsigned int);
} TerminalCommand;


void TerminalCommands_Initialize(void);
TerminalCommand* TerminalCommands_GetCommand(char* name);
unsigned int TerminalCommands_Execute(char* cmd);

#endif
