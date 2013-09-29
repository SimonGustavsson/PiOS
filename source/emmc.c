#include "emmc.h"
#include "mailbox.h"
#include "timer.h"
#include "stringutil.h"

volatile Emmc* gEmmc;
sd_scr gScr;
unsigned int gEmmcUseDMA = 0;
unsigned int gLastResp0;
unsigned int gLastResp1;
unsigned int gLastResp2;
unsigned int gLastResp3;

unsigned int gBlockSize;
unsigned int gBlocksToTransfer;
unsigned int* gSdBuffer;

// Support for unaligned data access
static inline void write_word(unsigned int val, unsigned short *buf, int offset)
{
    buf[offset + 0] = val & 0xff;
    buf[offset + 1] = (val >> 8) & 0xff;
    buf[offset + 2] = (val >> 16) & 0xff;
    buf[offset + 3] = (val >> 24) & 0xff;
}

static inline unsigned int read_word(unsigned short*buf, int offset)
{
	unsigned int b0 = buf[offset + 0] & 0xff;
	unsigned int b1 = buf[offset + 1] & 0xff;
	unsigned int b2 = buf[offset + 2] & 0xff;
	unsigned int b3 = buf[offset + 3] & 0xff;

	return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}


// Predefine the commands for ease of use
static int ACMD[] = {	SD_CMD_RESERVED(0),
	SD_CMD_RESERVED(1),
	SD_CMD_RESERVED(2),
	SD_CMD_RESERVED(3),
	SD_CMD_RESERVED(4),
	SD_CMD_RESERVED(5),
	SD_COMMAND_INDEX(6) | SD_RESP_R1 | IS_APP_CMD,
	SD_CMD_RESERVED(7),
	SD_CMD_RESERVED(8),
	SD_CMD_RESERVED(9),
	SD_CMD_RESERVED(10),
	SD_CMD_RESERVED(11),
	SD_CMD_RESERVED(12),
	SD_COMMAND_INDEX(13) | SD_RESP_R1 | IS_APP_CMD,
	SD_CMD_RESERVED(14),
	SD_CMD_RESERVED(15),
	SD_CMD_RESERVED(16),
	SD_CMD_RESERVED(17),
	SD_CMD_RESERVED(18),
	SD_CMD_RESERVED(19),
	SD_CMD_RESERVED(20),
	SD_CMD_RESERVED(21),
	SD_COMMAND_INDEX(22) | SD_RESP_R1 | SD_DATA_READ | IS_APP_CMD,
	SD_COMMAND_INDEX(23) | SD_RESP_R1 | IS_APP_CMD,
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
	SD_COMMAND_INDEX(41) | SD_RESP_R3 | IS_APP_CMD,
	SD_COMMAND_INDEX(42) | SD_RESP_R1 | IS_APP_CMD,
	SD_CMD_RESERVED(43),
	SD_CMD_RESERVED(44),
	SD_CMD_RESERVED(45),
	SD_CMD_RESERVED(46),
	SD_CMD_RESERVED(47),
	SD_CMD_RESERVED(48),
	SD_CMD_RESERVED(49),
	SD_CMD_RESERVED(50),
	SD_COMMAND_INDEX(51) | SD_RESP_R1 | SD_DATA_READ | IS_APP_CMD
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

char* gInterruptErrors[] = { "Command not finished", "Data not done", "block gap", "", "Write ready", "Read ready", "", "", "card interrupt request",
	"", "", "", "Retune", "Boot acknowledge", "Boot operation terminated", "An error meh", "Command line timeout", "Command CRC Error", "Command line end bit not 1", "Incorrect command index", "Timeout on data line", "data CRC Error", 
	"End bit on data line not 1", "", "Auto cmd error", "", "", "", "", "", "", ""};

void PrintErrorsInInterruptRegister(unsigned int reg)
{
	printf("[ERROR] ssed - send - Command failed to complete (interrupt:%d) ", reg);
	unsigned int i;
	for(i = 0; i < 32; i++)
	{
		if(reg & (1 << i) && strlen(gInterruptErrors[i]) > 0 && i != 15) // 15 = "error has occured", only show the detailed on(there is always one?)
		{
			printf("%s.\n", gInterruptErrors[i]);
			return;
		}
	}
	printf("Unknown error. :-(\n");
}
	
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
	gEmmc = (Emmc*)EMMC_BASE;

	// Power cycle to ensure initial state
	EmmcPowerCycle();

	printf("ssed - Version: %d Vendor: %d SdVersion: %d Slot status: %d\n",
		gEmmc->SlotisrVer.raw, 
		gEmmc->SlotisrVer.bits.vendor, 
		gEmmc->SlotisrVer.bits.sdversion, 
		gEmmc->SlotisrVer.bits.slot_status);
	

	gEmmc->Control1.raw |= 1 << 24;

	// Wait for the circuit to reset
	while((gEmmc->Control1.raw & (0x7 << 24)) != 0);

	// Wait for a card to be detected (should always be the case on the pi)
	while((gEmmc->Status.raw & (1 << 16)) != (1 << 16));

	gEmmc->Control2.raw = 0;

	unsigned int base_clock = Mailbox_SD_GetBaseFrequency();
	if(base_clock == -1)
		base_clock = 100000000;

	printf("ssed - Base clock speed: %d.\n", base_clock);

	unsigned int ctrl1 = gEmmc->Control1.raw;
	ctrl1 |= 1; // Enable clock
	ctrl1 |= Emmc_GetClockDivider(base_clock, 400000);
	ctrl1 |= (7 << 16); // Data timeout TMCLK * 2^10

	gEmmc->Control1.raw = ctrl1;

	while((gEmmc->Control1.raw & 0x2) != 0x2);

	wait(50);

	gEmmc->Control1.raw |= 4; // Enable the clock

	wait(50);

	gEmmc->IrptEn.raw = 0; // Disable ARM interrupts
	gEmmc->Interrupt.raw = 0xFFFFFFFF;
	gEmmc->IrptMask.raw = 0x17F7137;

	wait(200);

	EmmcSendCommand(CMD[GoIdleState], 0); // CMD0, should add timeout?

	EmmcSendCommand(CMD[SendIfCond], 0x1AA); // CMD8 Check if voltage is supported. Check pattern = 0xAA

	if((gLastResp0 & 0xFFF) != 0x1AA)
	{
		printf("ssed - Card version is not >= 2.0.\n");
		return -1;
	}

	EmmcSendCommand(ACMD[41], 0);

	unsigned int acmd41arg = 0x00ff8000 | (1 << 24) | (1 << 28) | (1 << 30);

	while(1)
	{
		EmmcSendCommand(ACMD[41], acmd41arg);

		wait(20);

		if((gLastResp0 >> 31) & 0x1)
			break;
		
		wait(20);
	}

	unsigned int sdhiCompatibility = (gLastResp0 >> 30) & 0x1;
	unsigned int ocr = (gLastResp0 >> 8) & 0xFFFF;

	printf("ssed - SDHI compatibility: %d ocr: %d.\n", sdhiCompatibility, ocr);

	EmmcSendCommand(CMD[2], 0x0); // ALL SEND CID

	wait(20);

	printf("ssed - Got CID: %d %d %d %d\n", gLastResp3, gLastResp2, gLastResp1, gLastResp0);

	EmmcSendCommand(CMD[3], 0); // SEND_RELATIVE_ADDR 1699

	unsigned int cmd3_resp = gLastResp0;

	printf("ssed - Relative address: %d.\n", cmd3_resp);

	unsigned int rca = (cmd3_resp >> 16) & 0xFFFF;
	unsigned int crc_error = (cmd3_resp >> 15) & 0x1;
	unsigned int illegal_cmd = (cmd3_resp >> 14) & 0x1;
	unsigned int error = (cmd3_resp >> 13) & 0x1;
	unsigned int status = (cmd3_resp >> 9) & 0xF;
	unsigned int ready = (cmd3_resp >> 8) & 0x1;
	
	if(crc_error)
	{
		printf("ssed - CMD3 CRC error.\n");
		return -1;
	}


	if(illegal_cmd)
	{
		printf("ssed - CMD3 illegal cmd.\n");
		return -1;
	}


	if(error)
	{
		printf("ssed - CMD3 generic error.\n");
		return -1;
	}


	if(!ready)
	{
		printf("ssed - CMD3 not ready for data.\n");
		return -1;
	}

	printf("ssed - RCA: %d\n", rca);

	// Not select the card to toggle it to transfer state
	EmmcSendCommand(CMD[7], rca << 16);

	unsigned int cmd7_resp = gLastResp0;

	status = (cmd7_resp >> 9) & 0xF;
	if((status != 3) && (status != 4))
	{
		printf("ssed - Invalid status: %d.\n", status);
		return -1;
	}

	printf("ssed - Getting SCR register.\n");
	
	// This is a data command, so setup the buffer
	gSdBuffer = &(gScr.scr[0]);
	gBlockSize = 8;
	gBlocksToTransfer = 1;
	
	EmmcSendCommand(ACMD[51], 0);

	// Set back to default
	gBlockSize = 512;

	// Determine card version
	//unsigned int scr0 = 

	return 0;
}

unsigned int EmmcSendCommand(unsigned int cmd, unsigned int argument)
{
	// Wait for command line to become available
	while(gEmmc->Status.bits.CmdInhibit == 1)
		wait(50);

	// If the response type is "With busy" and the command is not an abort command
	// We have to wait for the data line to become available before sending the command
	if((cmd & SD_CMD_RSPNS_TYPE_MASK) == SD_CMD_RSPNS_TYPE_48B && (cmd & SD_CMD_TYPE_ABORT) != SD_CMD_TYPE_ABORT)
	{	
		while(gEmmc->Status.bits.DatInhibit != 0)
			wait(50);
	}
	
	// Write block size count to register
	gEmmc->BlockCountSize.bits.BlkCnt = 1; // Sending one block, NOTE THIS WILL NEED CHANGING WITH OTHER COMMANDS
	gEmmc->BlockCountSize.bits.BlkSize = 512;

	unsigned int fixed_cmd;
	if(cmd & IS_APP_CMD)
	{ 		
		fixed_cmd = cmd - IS_APP_CMD;
		fixed_cmd |= 0x0 << 22; // Normal command type
		//cmd &= 0xFF; // TODO: Why do we need to set this? CRC
		if(!EmmcSendCommand(CMD[55], 0))
			return -1;
		
		wait(50);
	}
	
	// TODO: Handle DMA

	
	// Set arg1
	gEmmc->Arg1 = argument;

	// Finally - Write the command
	gEmmc->Cmdtm.raw = cmd;

	// Just relax for a bit, get a drink or something
	wait(2); 
	
	// Wait for the command done flag (or error :()
	while(gEmmc->Interrupt.bits.cmd_done == 0 && gEmmc->Interrupt.bits.err == 0)
		wait(20);

	unsigned int interrupt_reg = gEmmc->Interrupt.raw;

	// Clear the command complete status interrupt
	gEmmc->Interrupt.raw = 0xffff0001; // Clear "Command finished" and all status bits 860

	// Now check if we encountered an error sending our command
	if((interrupt_reg & 0xffff0001) != 0x1)
	{
		PrintErrorsInInterruptRegister(interrupt_reg);
		return -1;
	}

	switch(cmd & SD_CMD_RSPNS_TYPE_MASK)
	{
		case SD_CMD_RSPNS_TYPE_NONE:
			gEmmc->Resp0 = 0;
			gEmmc->Resp1 = 0;
			gEmmc->Resp2 = 0;
			gEmmc->Resp3 = 0;
			break;
		case SD_CMD_RSPNS_TYPE_48:
			gLastResp0 = gEmmc->Resp0;
			gEmmc->Resp0 = 0;
			break;
		case SD_CMD_RSPNS_TYPE_48B:
			gLastResp0 = gEmmc->Resp0;
			gEmmc->Resp0 = 0;
			break;
		case SD_CMD_RSPNS_TYPE_136:
			gLastResp0 = gEmmc->Resp0;
			gLastResp1 = gEmmc->Resp1;
			gLastResp2 = gEmmc->Resp2;
			gLastResp3 = gEmmc->Resp3;
			break;
	}

	if(cmd & SD_CMD_ISDATA)
	{
		unsigned int irpt;
		unsigned int is_write = 0;
		if(cmd & SD_CMD_DAT_DIR_CH)
			irpt = (1 << 5); // Read
		else
		{
			is_write = 1;
			irpt = (1 << 4);
		}

		unsigned int current_block = 0;
		unsigned int* buf = &gSdBuffer[0];

		while(current_block < gBlocksToTransfer)
		{
			if(gBlocksToTransfer > 1)
				printf("ssed - send - Waiting for block %d to get ready.\n", current_block);

			// Wait for block
			while(!(gEmmc->Interrupt.raw & (irpt | 0x8000)));
			unsigned int interrupts = gEmmc->Interrupt.raw;

			gEmmc->Interrupt.raw = 0xFFFF0000 | irpt;

			if((interrupts & (0xFFFF0000 | irpt)) != irpt)
			{
				printf("ssed - send - Error occurred whilst waiting for data ready interrupt.\n");
				return -1; // sadness, failed to read
			}

			// Transfer the block
			unsigned int current_byte_number = 0;
			while(current_byte_number < gBlockSize)
			{
				if(is_write)
				{
					gEmmc->Data = read_word((unsigned short*)buf, 0);
				}
				else
				{
					write_word(gEmmc->Data, (unsigned short*)buf, 0);
				}
				current_byte_number += 4;
				buf++;
			}

			printf("ssed - send - Block transfer complete.\n");
			current_block ++;
		}
	}

	// Wait for transfer complete (set if read/write transfer with busy)
	if((((cmd & SD_CMD_RSPNS_TYPE_MASK) == SD_CMD_RSPNS_TYPE_48B) ||
		(cmd & SD_CMD_ISDATA)))
	{
		// First make sure DAT Is not already 0
		if(gEmmc->Status.bits.DatInhibit == 0)
		{
			gEmmc->Interrupt.raw = 0xFFFF0002;
		}
		else
		{
			// Wait for the interrupt
			printf("ssed - send - Waiting for transfer complete interrupt.\n");
			while(!(gEmmc->Interrupt.raw & 0x8002));
			printf("ssed - send - Got transfer complete interrupt!\n");

			interrupt_reg = gEmmc->Interrupt.raw;

			gEmmc->Interrupt.raw = 0xFFFF0002;

			if(((interrupt_reg & 0xFFFF0002) != 0x2) && ((interrupt_reg & 0xFFFF0002) != 0x100002))
			{
				printf("ssed - send - Error occured whilst waiting for transfer complete interrupt.\n");
				return -1;
			}
			gEmmc->Interrupt.raw = 0xFFFF0002;
		}
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
