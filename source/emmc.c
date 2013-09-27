#include "emmc.h"
#include "mailbox.h"
#include "timer.h"
#include "stringutil.h"

volatile Emmc* gEmmc;
unsigned int gEmmcUseDMA = 0;

// Predefine the commands for ease of use
static int ACMD[] = {
	SD_CMD_RESERVED(0),
	SD_CMD_RESERVED(1),
	SD_CMD_RESERVED(2),
	SD_CMD_RESERVED(3),
	SD_CMD_RESERVED(4),
	SD_CMD_RESERVED(5),
	SD_COMMAND_INDEX(6) | SD_RESP_R1,
	SD_CMD_RESERVED(7),
	SD_CMD_RESERVED(8),
	SD_CMD_RESERVED(9),
	SD_CMD_RESERVED(10),
	SD_CMD_RESERVED(11),
	SD_CMD_RESERVED(12),
	SD_COMMAND_INDEX(13) | SD_RESP_R1,
	SD_CMD_RESERVED(14),
	SD_CMD_RESERVED(15),
	SD_CMD_RESERVED(16),
	SD_CMD_RESERVED(17),
	SD_CMD_RESERVED(18),
	SD_CMD_RESERVED(19),
	SD_CMD_RESERVED(20),
	SD_CMD_RESERVED(21),
	SD_COMMAND_INDEX(22) | SD_RESP_R1 | SD_DATA_READ,
	SD_COMMAND_INDEX(23) | SD_RESP_R1,
	SD_CMD_RESERVED(24),
	SD_CMD_RESERVED(25),
	SD_CMD_RESERVED(26),
	SD_CMD_RESERVED(27),
	SD_CMD_RESERVED(28),
	SD_CMD_RESERVED(29),
	SD_CMD_RESERVED(30),
	SD_CMD_RESERVED(31),
	SD_CMD_RESERVED(32),
	SD_CMD_RESERVED(33),
	SD_CMD_RESERVED(34),
	SD_CMD_RESERVED(35),
	SD_CMD_RESERVED(36),
	SD_CMD_RESERVED(37),
	SD_CMD_RESERVED(38),
	SD_CMD_RESERVED(39),
	SD_CMD_RESERVED(40),
	SD_COMMAND_INDEX(41) | SD_RESP_R3,
	SD_COMMAND_INDEX(42) | SD_RESP_R1,
	SD_CMD_RESERVED(43),
	SD_CMD_RESERVED(44),
	SD_CMD_RESERVED(45),
	SD_CMD_RESERVED(46),
	SD_CMD_RESERVED(47),
	SD_CMD_RESERVED(48),
	SD_CMD_RESERVED(49),
	SD_CMD_RESERVED(50),
	SD_COMMAND_INDEX(51) | SD_RESP_R1 | SD_DATA_READ
};

static int CMD[] = {
	SD_COMMAND_INDEX(0)  | SD_RESP_NONE,
	SD_CMD_RESERVED(1),			 
	SD_COMMAND_INDEX(2)  | SD_RESP_R2,
	SD_COMMAND_INDEX(3)  | SD_RESP_R6,
	SD_COMMAND_INDEX(4)  | SD_RESP_NONE,
	SD_COMMAND_INDEX(5)  | SD_RESP_R4,
	SD_COMMAND_INDEX(6)  | SD_RESP_R1,
	SD_COMMAND_INDEX(7)  | SD_RESP_R1b,
	SD_COMMAND_INDEX(8)  | SD_RESP_R7,
	SD_COMMAND_INDEX(9)  | SD_RESP_R2,
	SD_COMMAND_INDEX(10) | SD_RESP_R2,
	SD_COMMAND_INDEX(11) | SD_RESP_R1,
	SD_COMMAND_INDEX(12) | SD_RESP_R1b | SD_CMD_TYPE_ABORT, 
	SD_COMMAND_INDEX(13) | SD_RESP_R1,
	SD_CMD_RESERVED(14),			 
	SD_COMMAND_INDEX(15) | SD_RESP_NONE,
	SD_COMMAND_INDEX(16) | SD_RESP_R1,
	SD_COMMAND_INDEX(17) | SD_RESP_R1 | SD_DATA_READ,
	SD_COMMAND_INDEX(18) | SD_RESP_R1 | SD_DATA_READ | SD_CMD_MULTI_BLOCK,
	SD_COMMAND_INDEX(19) | SD_RESP_R1 | SD_DATA_READ,
	SD_COMMAND_INDEX(20) | SD_RESP_R1b,
	SD_CMD_RESERVED(21),			 
	SD_CMD_RESERVED(22),			 
	SD_COMMAND_INDEX(23) | SD_RESP_R1,
	SD_COMMAND_INDEX(24) | SD_RESP_R1 | SD_DATA_WRITE,
	SD_COMMAND_INDEX(25) | SD_RESP_R1 | SD_DATA_WRITE | SD_CMD_MULTI_BLOCK,
	SD_CMD_RESERVED(26),			 
	SD_COMMAND_INDEX(27) | SD_RESP_R1 | SD_DATA_WRITE,
	SD_COMMAND_INDEX(28) | SD_RESP_R1b,
	SD_COMMAND_INDEX(29) | SD_RESP_R1b,
	SD_COMMAND_INDEX(30) | SD_RESP_R1b | SD_DATA_READ,
	SD_CMD_RESERVED(31),
	SD_COMMAND_INDEX(32) | SD_RESP_R1,
	SD_COMMAND_INDEX(33) | SD_RESP_R1,
	SD_CMD_RESERVED(34),
	SD_CMD_RESERVED(35),
	SD_CMD_RESERVED(36),
	SD_CMD_RESERVED(37),
	SD_COMMAND_INDEX(38) | SD_RESP_R1b,
	SD_CMD_RESERVED(39),
	SD_CMD_RESERVED(40),
	SD_CMD_RESERVED(41),
	SD_COMMAND_INDEX(42) | SD_RESP_R1,
	SD_CMD_RESERVED(43),
	SD_CMD_RESERVED(44),
	SD_CMD_RESERVED(45),
	SD_CMD_RESERVED(46),
	SD_CMD_RESERVED(47),
	SD_CMD_RESERVED(48),
	SD_CMD_RESERVED(49),
	SD_CMD_RESERVED(50),
	SD_CMD_RESERVED(51),
	SD_CMD_RESERVED(52),
	SD_CMD_RESERVED(53),
	SD_CMD_RESERVED(54),
	SD_COMMAND_INDEX(55) | SD_RESP_R1,
	SD_COMMAND_INDEX(56) | SD_RESP_R1 | SD_CMD_ISDATA
};
	
unsigned int Emmc_GetClockDivider(unsigned int base_clock, unsigned int target_rate)
{
	// MATH - HOW DOES IT WORK!?
	unsigned int targetted_divisor = 0;
	if(target_rate > base_clock)
		targetted_divisor = 1;
	else
	{
		targetted_divisor = base_clock / target_rate;
		if(base_clock % target_rate)
			targetted_divisor--;
	}

	// TODO: Decide the clock mode to use, currently only 10-bit divided clock mode is supported
	
	// Find the first set bit 
	unsigned int divisor = -1;
	unsigned int first_bit;
	for(first_bit = 31; first_bit >= 0; first_bit--)
	{
		unsigned int bit_test = (1 << first_bit);
		if(targetted_divisor & bit_test)
		{
			divisor = first_bit;
			targetted_divisor &= ~bit_test;
			if(targetted_divisor)
				divisor++; // The divisor is not power of two, increase to fix

			break; // Found it!
		}
	}

	if(divisor == -1)
		divisor = 31;
	if(divisor >= 32)
		divisor = 31;

	if(divisor != 0)
		divisor = (1 << (divisor -1));

	if(divisor >= 0x400)
		divisor = 0x3FF;

	unsigned int freq_select = divisor & 0xFF;
	unsigned int upper_bits = (divisor >> 8) & 0x3;
	unsigned int ret = (freq_select << 8) | (upper_bits << 6);

	// For debugging
	int denominator = -1;
	if(divisor != 0)
		denominator = divisor * 2;
	int actual_clock = base_clock / denominator;
	printf("ssed - Base clock: %d, target rate: %d, divisor: %d, actual clock: %d, ret: %d\n",
		base_clock, target_rate, divisor, actual_clock, ret);

	return ret;
}

unsigned int EmmcInitialise(void)
{
	printf("ssed - Initializing external mass media controller.\n");
	
	gEmmc = (Emmc*)EMMC_BASE;

	// Power cycle to ensure initial state
	EmmcPowerCycle();

	printf("ssed - Version: %d Vendor: %d SdVersion: %d Slot status: %d\n",
		gEmmc->SlotisrVer.raw, 
		gEmmc->SlotisrVer.bits.vendor, 
		gEmmc->SlotisrVer.bits.sdversion, 
		gEmmc->SlotisrVer.bits.slot_status);
	
	printf("ssed - Resetting circuit... ");

	// Reset the entire host circuit
	gEmmc->Control1.bits.srst_hc = 1;

	// Disable internal clock
	gEmmc->Control1.bits.clk_intlen = 0;

	// Disable SD clock
	gEmmc->Control1.bits.clk_en = 0;

	// Wait for controller to reset 
	//WARNING: If reset fails we will hang forever, but it's K (for now) because we're showing a message
	while(gEmmc->Control1.bits.clk_en != 0 || gEmmc->Control1.bits.srst_data != 0 || gEmmc->Control1.bits.srst_hc != 0) continue;
	
	printf("Done!\n");

	printf("ssed - Control0: %d.\n", gEmmc->Control0.raw);
	printf("ssed - Control1: %d.\n", gEmmc->Control1.raw);
	printf("ssed - Control2: %d.\n", gEmmc->Control2.raw);
	
	printf("ssed - Capabilities 0: %d.\n", gEmmc->reserved);
	printf("ssed - Capabilities 1: %d.\n", gEmmc->reserved2);

	// TODO: Check if card is actually inserted (bit 16 of Status register?) 
	//(Not doing this now as it should always be in)
	printf("ssed - Status: %d.\n", gEmmc->Status.raw);

	// Clear control2
	gEmmc->Control2.raw = 0;

	// Get clock rate from mailbox 1380
	unsigned int base_clock = Mailbox_SD_GetBaseFrequency();
	
	if(base_clock == -1)
	{
		printf("ssed - Failed to retrieve base clock, assuming 100MHz.\n");
		base_clock = 100000000;
	}

	// Enable the internal mmc clock
	gEmmc->Control1.bits.clk_intlen = 1; 

	// During identification we set the clock to something slow
	// Get clock divider
	unsigned int identificationFrequency = Emmc_GetClockDivider(base_clock, SdClockId);

	if(identificationFrequency == -1)
	{
		printf("ssed - Faild to get clock divider.\n");
		return -1;
	}

	printf("ssed - Identification frequency: %d.\n", identificationFrequency);

	// Set the frequency and data timeout
	gEmmc->Control1.bits.clk_freq8 = identificationFrequency;
	gEmmc->Control1.bits.data_taunit = 7; // TMCLK * 2^10 TODO: WAT?

	// Wait for clock to stabalize
	printf("ssed - Waiting for clock to stabalize...");
	while(gEmmc->Control1.bits.clk_stable != 1) continue;
	printf(" Done!\n");

	// Enable SD clock
	printf("ssed - Enabling SD Clock.\n");
	gEmmc->Control1.bits.clk_en = 1;

	wait(1000);

	// TODO: Setup interrupts

	// Send CMD 0
	printf("ssed - Sending 'Go Idle State'.\n");

	if(EmmcSendCommand(CMD[GoIdleState], 0) != 1)
	{
		printf("CMD0 (GO_IDLE) failed. :-(\n");
		return -1;
	}

	printf("ssed - Sending 'Go Idle State' Success.\n");

	// 0x1AA = 0x1 2.7-3.6V (Standard, 0xAA check pattern
	if(EmmcSendCommand(CMD[SendIfCond], 0x1AA) != 1)
	{
		printf("CMD8 (SEND_IF_COND) failed.\n");
		return -1;
	}

	// Hmm, Send CMD5? Not sure if applicable - are SDIO cards relevant?

	// Send ACMD41 to get OCR (voltage window = 0)
	if(EmmcSendCommand(ACMD[41], 0) != 1)
	{
		printf("ACMD41 Failed. \n");
		return -1;
	}

	printf("ACMD41 returned: %d", gEmmc->Resp0);

	printf("ssed - Initialization completed, so far so good.\n");

	unsigned int card_isbusy = 1;
	while(card_isbusy)
	{
		// Send initialization ACMD41
	}


	// Set voltage to 1.8v and enable max performance


	// After all is over and done with, switch clock speed to normal
	// Wait a little bit and then switch to 1.8V if possible


	// Send CMD3 to enter data state
	// CMD7 to toggle to transfer state
	return 0;
}

unsigned int EmmcSendCommand(unsigned int cmd, unsigned int argument)
{
	if(cmd & IS_APP_CMD)
	{ 
		printf("ssed - send - Sending app command: '%d', with CRC: '%d'.\n", cmd, cmd & 0xFF);
		
		cmd &= 0xFF; // TODO: Why do we need to set this? CRC
		if(cmd == SD_CMD_RESERVED(cmd))
			return -1; // Invalid command
	}
	
	printf("ssed - send - waiting for command line to become available.\n");

	// Wait for command line to become available
	while(gEmmc->Status.bits.CmdInhibit == 1)
		wait(50);

	printf("ssed - send - Command line available.\n");

	// If the response type is "With busy" and the command is not an abort command
	// We have to wait for the data line to become available before sending the command
	if((cmd & SD_RESP_R1b) == SD_RESP_R1b && (cmd & SD_CMD_TYPE_ABORT) != SD_CMD_TYPE_ABORT)
	{
		printf("ssed - send - command response is R1b and ABORT, waiting for dataline...\n");
		
		while(gEmmc->Status.bits.DatInhibit != 0)
			wait(50);

		printf("ssed - send - Data line available.\n");
	}

	// TODO: Handle DMA

	printf("ssed - send - Setting blocksize count(1) and size(512).\n");

	// Write block size count to register
	gEmmc->BlockCountSize.bits.BlkCnt = 1; // Sending one block, NOTE THIS WILL NEED CHANGING WITH OTHER COMMANDS
	gEmmc->BlockCountSize.bits.BlkSize = 512;

	printf("ssed - send - Setting argument: '%d'.\n", argument);

	// Set arg1
	gEmmc->Arg1 = argument;

	// Finally - Write the command

	printf("ssed - send - writing command '%d', before: '%d'.\n", cmd, gEmmc->Cmdtm.raw);

	gEmmc->Cmdtm.raw = cmd; // 835

	printf("ssed - send - writing command '%d', after: '%d'.\n", cmd, gEmmc->Cmdtm.raw);

	// Just relax for a bit, get a drink or something
	wait(200); 

	printf("ssed - send - waitin for command done flag (or error)\n");

	// Wait for the command done flag (or error :()
	while(gEmmc->Interrupt.bits.cmd_done != 1 || gEmmc->Interrupt.bits.err != 0)
		wait(20);

	printf("ssed - send - Command done! (or error) :(\n");

	// Clear the command complete status interrupt
	gEmmc->Interrupt.raw = 0xffff0001; // Clear "Command finished" and all status bits 860

	// Now check if we encountered an error sending our command
	if((gEmmc->Interrupt.raw & 0xffff0001) != 1)
	{
		printf("ssed - send - Error while waiting for command complete flag in interrupt register :(\n");
		return -1;
	}

	printf("ssed - send - Command executed successfully.\n");

	switch(cmd & SD_CMD_RSPNS_TYPE_MASK)
	{
		case SD_RESP_NONE:
			break;
		case SD_RESP_R1:
		case SD_RESP_R1b:
			break;
		case SD_RESP_R2:
			// Response is in Resp0 through Resp3
			break;
	}

	// If the command was with data, wait for the appropriate interrupt
	if((cmd & SD_RESP_R1b) == SD_RESP_R1b || 0 == 1) // 0 == 1 should be "is data transfer"
	{
		// Check if DAT is not already 0
		if(gEmmc->Status.bits.DatInhibit == 0)
		{
			// Write 0xffff0002 to interrupt? :S Clear it again I guess?
		}
		else
		{
			while(gEmmc->Interrupt.bits.cmd_done != 0 || gEmmc->Interrupt.bits.err == 0)
				wait(20);

			// TODO: Handle case where both data timeout and transfer complete are set - transfer complete overrides data timeout 966
		}
	}
	else if(0 == 1) // is_dma
	{
		// For SDMA transfers, we have to wait for either transfer complete,
        //  DMA int or an error
	}

	// Success!
	return 1;	
}

unsigned int EmmcPowerOn(void)
{
	return Mailbox_SetDevicePowerState(0x0, 1);
}

unsigned int EmmcPowerOff(void)
{
	return Mailbox_SetDevicePowerState(0x0, 0);
}

unsigned int EmmcPowerCycle(void)
{
	printf("ssed - Power cycling: ");
	
	unsigned int res = 0;
	if((res = EmmcPowerOff()) < 0)
	{
		printf("Failed!\n");
		return -1;
	}	
	
	wait(50);
	
	if((res = EmmcPowerOn()) < 0)
		printf("Failed!\n");
	else
		printf("Success!\n");
	
	return EmmcPowerOn();
}
