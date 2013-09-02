#ifndef TERMINAL_COMMANDS
#define TERMINAL_COMMANDS

typedef struct
{
	char* name;
	unsigned int (*execute)(char**, unsigned int);
} TerminalCommand;

void TerminalInitCommands(void);
TerminalCommand* TerminalGetCommand(char* name);

#endif
