#define MAX_COMMAND_COUNT 20
#define MAX_COMMAND_ARGUMENT_COUNT 10
#include "terminalCommands.h"
#include "terminal.h"
#include "types/string.h"
#include "types/types.h"
#include "hardware/mailbox.h"
#include "hardware/emmc.h"

extern char gPrompt[TERMINAL_PROMPT_MAX_LENGTH];
TerminalCommand gCommands[MAX_COMMAND_COUNT];
unsigned int gCommandCount = 0;

extern volatile Emmc* gEmmc;

void TerminalCommands_registerCommand(char* name, char* description, unsigned int (*execute)(char**, unsigned int))
{
	gCommands[gCommandCount].name = name;
	gCommands[gCommandCount].description = description;
	gCommands[gCommandCount].execute = execute;
	
	gCommandCount++;
}

TerminalCommand *TerminalCommands_GetCommand(char* name)
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
		TerminalCommand* cmd = TerminalCommands_GetCommand(args[1]);
		
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
	Terminal_Clear();
	
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

unsigned int Command_DbgSd_Execute(char** args, unsigned int argCount)
{
	printf("gEmmc addr: %d\n", (unsigned int)gEmmc);
	printf("Arg2 = %d\n", gEmmc->Arg2);
	printf("BlockCountSize = %d\n", gEmmc->BlockCountSize.raw);
	printf("Arg1 = %d\n", gEmmc->Arg1);
	printf("Cmdtm = %d\n", gEmmc->Cmdtm.raw);
	printf("Resp0 = %d\n", gEmmc->Resp0);
	
	printf("Resp1 = %d\n", gEmmc->Resp1);
	printf("Resp3 = %d\n", gEmmc->Resp3);
	printf("Data = %d\n", gEmmc->Data);
	printf("Status = %d\n", gEmmc->Status.raw);
	printf("Control0 = %d\n", gEmmc->Control0.raw);
	
	printf("Control1 = %d\n", gEmmc->Control1.raw);
	printf("Interrupt = %d\n", gEmmc->Interrupt.raw);
	printf("IrptMask = %d\n", gEmmc->IrptMask.raw);
	printf("Control2 = %d\n", gEmmc->Control2.raw);
	printf("ForceIrpt = %d\n", gEmmc->ForceIrpt.raw);
	
	printf("BootTimeout = %d\n", gEmmc->BootTimeout);
	printf("DbgSel = %d\n", gEmmc->DbgSel.raw);
	printf("ExrdfifoCfg = %d\n", gEmmc->ExrdfifoCfg.raw);
	printf("ExrdfifoEnable = %d\n", gEmmc->ExrdfifoEnable.raw);
	
	printf("TuneStep = %d\n", gEmmc->TuneStep);
	printf("TuneStepsStd = %d\n", gEmmc->TuneStepsStd.raw);
	printf("TuneStepsDdr = %d\n", gEmmc->TuneStepsDdr.raw);
	printf("SpiIntSpt = %d\n", gEmmc->SPIIntSpt.raw);
	printf("SlotisrVer = %d\n", gEmmc->SlotisrVer.raw);
	
	return 0;
}

unsigned int Command_SdStatus_Execute(char** args, unsigned int argCount)
{
	printf("Can write: %s", gEmmc->Status.bits.WriteTransfer == 1 ? "True" : "False");
	printf("Can read: %s", gEmmc->Status.bits.ReadTransfer == 1 ? "True" : "False");

	return 0;
}

unsigned int Command_SdControl0_Execute(char** args, unsigned int argCount)
{
	printf("EMMC - Control0:\n");
	printf("Use 4 data lines: %s\n", gEmmc->Control0.bits.hctl_dwidth == 1 ? "Yes" : "No");
	printf("Select high-speed mode: %s\n", gEmmc->Control0.bits.hctl_hs_en == 1 ? "Yes" : "No");
	printf("Use 8 data lines: %s\n", gEmmc->Control0.bits.hctl_8bit == 1 ? "Yes" : "No");
	printf("Stop the current transaction at BLOCK_GAP: %s\n", gEmmc->Control0.bits.gap_stop == 1 ? "Yes" : "No");
	printf("Restart when stopped at GAP_STOP: %s\n", gEmmc->Control0.bits.gap_restart == 1 ? "Yes" : "No");
	printf("Use DAT2 read-wait protocol: %s\n", gEmmc->Control0.bits.readwait_en == 1 ? "Yes" : "No");
	printf("Enable SDIO interrupt at BLOCK_GAP: %s\n", gEmmc->Control0.bits.gap_ien == 1 ? "Yes" : "No");
	printf("SPI Mode enabled: %s\n", gEmmc->Control0.bits.spi_mode == 1 ? "Yes" : "No");
	printf("Boot mode access: %s\n", gEmmc->Control0.bits.boot_en == 1 ? "Start boot mode access" : "Stop boot mode access");
	printf("Enable alternative boot mode access: %s\n", gEmmc->Control0.bits.alt_boot_en == 1 ? "Yes" : "No");

	return 0;
}

unsigned int Command_SetPower_Execute(char** args, unsigned int argCount)
{
	if(argCount != 3)
	{
		printf("Unexpected amount of parameters, type 'help setpower' for help.\n");
		return -1;
	}
	
	unsigned int deviceId = *args[1] - '0'; // Note: Just uses the first character
	if(deviceId < 0 || deviceId > 8)
	{
		printf("Invalid device id, expected values between 0 - 8.\n");
		return -1;
	}
	
	unsigned int powerState = *args[2] - '0'; // Note: Just uses the first character
	if(powerState < 0 || powerState > 1)
	{
		printf("Invalid power state, expected 0 (Off) or 1 (On).\n");
		return -1;
	}
	
	unsigned int res = 0;
	if((res = Mailbox_SetDevicePowerState(deviceId, powerState)) != 0)
		printf("Failed to set power for device '%d' to '%d'", deviceId, powerState);
	else
		printf("Sucess setting power of device '%d' to '%d'", deviceId, powerState);
	
	return res;	
}

unsigned int Command_GetPower_Execute(char** args, unsigned int argCount)
{
	if(argCount != 2)
	{
		printf("Unexpected amount of arguments, type 'help getpower' for help.\n");
		return -1;
	}
	
	unsigned int deviceId = *args[1] - '0';
	
	unsigned int powerState = Mailbox_GetPowerState(deviceId);
	
	if(powerState == -1)
	{
		printf("Failed to retrieve power state.\n");
		return -1;
	}
	
	printf("The power state of '%d' is '%d'", deviceId, powerState);
	
	return 0;
}

void TerminalCommands_Initialize(void)
{
	TerminalCommands_registerCommand("about", "Prints information about PiOS.", &Command_About_Execute);
    TerminalCommands_registerCommand("help", "Prints a listing of commands.", &Command_Help_Execute);
    TerminalCommands_registerCommand("test", "Command for testing new things.", &Command_Test_Execute);
    TerminalCommands_registerCommand("cls", "Clears the terminal.", &Command_Cls_Execute);
    TerminalCommands_registerCommand("echo", "Prints the given string to the terminal.", &Command_Echo_Execute);
    TerminalCommands_registerCommand("prompt", "Changes the system prompt text.", &Command_Prompt_Execute);
    TerminalCommands_registerCommand("setpower", "Changes the power state(1 or 0) of the given device(0-8).", &Command_SetPower_Execute);
    TerminalCommands_registerCommand("getpower", "Retrieves the power state of the given device (0-8).", &Command_GetPower_Execute);

	// Sd cards for debugging
    TerminalCommands_registerCommand("dbgsd", "Prints the mmc struct", &Command_DbgSd_Execute);
    TerminalCommands_registerCommand("sdstatus", "Prints the status of the external mass media controller", &Command_SdStatus_Execute);
    TerminalCommands_registerCommand("sdcontrol0", "Prints the current control0 configuration", &Command_SdControl0_Execute);
}

// 0: OK, -1: Unknown command
unsigned int TerminalCommands_Execute(char* cmd)
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
	
	TerminalCommand* command = TerminalCommands_GetCommand(arguments[0]);
	
	if(command == 0)
	{
		printf("command with name %s not found\n", arguments[0]);
		
		return -1; // Unknown command
	}
	
	return command->execute(arguments, argCount + 1); // Index to count, add one
}
