#include "hardware/emmc.h"
#include "hardware/mailbox.h"
#include "hardware/timer.h"
#include "types/string.h"
#include "util/utilities.h"

#define SDMA_BUFFER     0x6000
#define SDMA_BUFFER_PA  (SDMA_BUFFER + 0xC0000000)
#define SD_CMD_DMA          1
#define ACMD(a) (a | IS_APP_CMD)

volatile Emmc* gEmmc;
sd gDevice;
unsigned int gEmmcUseDMA = 0;

unsigned int gLastCommand = 0;
unsigned int gLastCommandSuccess = 0;
unsigned int gLastError = 0;
static unsigned int gHciVersion;
static unsigned int gSdCapabilities0;
static unsigned int gSdCapabilities1;

// Predefine the commands for ease of use
static int ACMD[] = { 
	SD_CMD_RESERVED(0),
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

char* gSdVersionStrings[] = { "Unknown", "Version 1/1.01", "Version 1.10", "Version 2.00", "Version 3.0x", "Version 4.xx"};

char* gInterruptErrors[] = { "Command not finished", "Data not done", "Block gap", "", "Write ready", "Read ready", "", "", "Card interrupt request",
	"", "", "", "Retune", "Boot acknowledge", "Boot operation terminated", "An error meh", "Command line timeout", "Command CRC Error", "Command line end bit not 1", "Incorrect command index", "Timeout on data line", "Data CRC Error", 
	"End bit on data line not 1", "", "Auto cmd error", "", "", "", "", "", "", ""};

void PrintErrorsInInterruptRegister(unsigned int reg)
{
	printf("[ERROR] ssed - send - Command failed to complete (interrupt:%d) ", reg);

	// Start at 1 to skip "Command not finished" message
	unsigned int i;
	for(i = 1; i < 32; i++)
	{
		if(reg & (1 << i) && strlen(gInterruptErrors[i]) > 0 && i != 15) // 15 = "error has occured", only show the detailed on(there is always one?)
		{
			printf("%d. %s.\n", i, gInterruptErrors[i]);
			return;
		}
	}
	printf("Unknown error. :-(\n");
}
	
unsigned int Emmc_getClockDivider(unsigned int base_clock, unsigned int target_rate)
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
	unsigned int ret = (freq_select << 8) | (upper_bits << 6) | (0 << 5);

	// For debugging
	//int denominator = -1;
	//if(divisor != 0)
	//	denominator = divisor * 2;
	//
	//int actual_clock = base_clock / denominator;

	return ret;
}

unsigned int Emmc_switchClockRate(unsigned int base_clock, unsigned int target_rate)
{
	unsigned int divider = Emmc_getClockDivider(base_clock, target_rate);
	if(divider == -1)
	{
		printf("ssed -  Failed to retrieve divider for target_rate: %d.\n", target_rate);
		return -1;
	}

	// Wait for command line, and DAT line to become available
	while(gEmmc->Status.bits.CmdInhibit == 1 || gEmmc->Status.bits.DatInhibit == 1)
		wait(100);

	// Turn the clock off
	gEmmc->Control1.raw &= ~(1 << 2);

	wait(200);

	// Write the new divider
	unsigned int control1 = gEmmc->Control1.raw;
	control1 &= ~0xFFE0; // Clear old clock generator select
	control1 |= divider;

	gEmmc->Control1.raw = control1;

	wait(100);

	// Re enable the SD clock with the new speed
	control1 |= (1 << 2);
	gEmmc->Control1.raw = control1;

	wait(200);

	return 1;
}

int Emmc_Initialise(void)
{
    printf("ssed - Initialising...\n");

	gEmmc = (Emmc*)EMMC_BASE;
	gDevice.rca = 0;
	gDevice.blocks_to_transfer = 0;
	gEmmcUseDMA = 0;

	// Power cycle to ensure initial state
	// TODO: Add error checking
    if (Emmc_powerCycle() != 0)
	{
		printf("ssed - Controller did not successfully power cycle.\n");
		return -1;
	}

	//printf("ssed - Controller power cycled.\n");

	gHciVersion = (gEmmc->SlotisrVer.raw >> 16) & 0xff ;
	
	/*printf("ssed - Version: %d Vendor: %d SdVersion: %d Slot status: %d\n",
		gEmmc->SlotisrVer.raw, 
		gEmmc->SlotisrVer.bits.vendor, 
		gEmmc->SlotisrVer.bits.sdversion, 
		gEmmc->SlotisrVer.bits.slot_status);*/
	
	if(gHciVersion < 2)
	{
		printf("ssed - Only SDHCI versions >= 3.0 are supported. %d is a bit shit innit.\n", gHciVersion);
		return -1;
	}
	
	// Disable clock
	unsigned int control1 = gEmmc->Control1.raw;
	control1 |= (1 << 24);
	control1 &= ~(1 << 2); // Disable clock
	control1 &= ~(1 << 0);
	gEmmc->Control1.raw = control1;
	
	// Wait for the circuit to reset
	while((gEmmc->Control1.raw & (0x7 << 24)) != 0) { /* Do Nothing */ }

	gSdCapabilities0 = gEmmc->Capabilities0;
	gSdCapabilities1 = gEmmc->Capabilities1;

	//printf("ssed - Capabilities: %d - %d.\n", gSdCapabilities0, gSdCapabilities1);

	// Wait for a card to be detected (should always be the case on the pi)
	while((gEmmc->Status.raw & (1 << 16)) != (1 << 16));
	
	gEmmc->Control2.raw = 0;

	// Get base clock rate
    unsigned int base_clock = Mailbox_SdGetBaseFrequency();
	if(base_clock == -1)
	{
		printf("ssed - Invalid base clock, assuming 100MHz.\n");
		base_clock = 100000000;
	}

	//printf("ssed - Base clock speed: %d.\n", base_clock);
	
	// Set identification frequency 400 kHz (this is then later increased to 25MHz)
	control1 = gEmmc->Control1.raw;
	control1 |= 1; // Enable clock


	unsigned int id_freq = Emmc_getClockDivider(base_clock, SdClockId);
	if(id_freq == -1)
	{
		printf("ssed - Unable to get valid clock divider for ID frequency.\n");
		return -1;
	}

	control1 |= id_freq;
	control1 |= (7 << 16); // Data timeout TMCLK * 2^10

	gEmmc->Control1.raw = control1;

	// Wait for the clock to stabalize
	while((gEmmc->Control1.raw & 0x2) != 0);

	wait(100);

	gEmmc->Control1.raw |= 4; // Enable the clock

	wait(100);

	gEmmc->IrptEn.raw = 0; // Disable ARM interrupts
	gEmmc->Interrupt.raw = 0xFFFFFFFF;
	unsigned int irpt_mask = 0xFFFFFFFF & (~SD_CARD_INTERRUPT);

	// ENABLE INTERRUPTS?
	irpt_mask |= SD_CARD_INTERRUPT;

	gEmmc->IrptMask.raw = irpt_mask;
	
	wait(100);

	if(!Emmc_issueCommand(GoIdleState, 0)) // CMD0, should add timeout?
	{
		printf("ssed - No CMD0 response.\n");
		return -1;
	}

	// CMD8 Check if voltage is supported. 
	// Voltage: 0001b (2.7-3.6V), Check pattern: 0xAA
	int v2_later = 0;
	if(!Emmc_issueCommand(SendIfCond, 0x1AA))
	{	
		printf("ssed - Send if cond failed.\n");
		return -1;
	}

	if((gDevice.last_resp0 & 0xFFF) != 0x1AA)
	{
		printf("ssed - Card version is not >= 2.0. CMD8 resp: %d.\n", gDevice.last_resp0 & 0xFFF);
		return -1;
	}

	v2_later = 1;

	// Send inquiry ACMD41 to get OCR
	if(!Emmc_issueCommand(ACMD(41), 0))
	{
		printf("ssed - Inquiry ACMD41 failed.\n");
		return -1;
	}
	
	unsigned int card_supports_voltage_switch = 0; //(gDevice.last_resp0 & (1 << 24));
	while(1)
	{
		unsigned int flags = 0;
		if(v2_later)
		{
			// Enable SDHC support
			flags |= (1 << 30);

			// Set 1.8v support
			if(!gDevice.failed_voltage_switch)
				flags |= (1 << 24);

			// Enable SDXC Maximum performance
			flags |= (1 << 28);
		}

		if(!Emmc_issueCommand(ACMD(41), 0x00FF8000 | flags))
		{
			printf("ssed - Error issuing ACMD41\n");
			return -1;
		}

		if((gDevice.last_resp0 >> 31) & 0x1)
		{
			gDevice.ocr = (gDevice.last_resp0 >> 8) & 0xFFFF;
			gDevice.supports_sdhc = (gDevice.last_resp0 >> 30) & 0x1;
			
			/*if((gDevice.last_resp0 >> 30) & 0x1)
				printf("ssed - card supports SDHC.\n");*/
			
			card_supports_voltage_switch = (gDevice.last_resp0 >> 24) & 0x1;

			break;
		}

		wait(500);
	}

	/* TODO: Store this for later use?
	if(((gDevice.last_resp0 >> 30) & 0x1) == 0)
		printf("ssed - card is a SDSC.\n");
	else
		printf("ssed - card is SDHC/SDXC.\n");
	*/
	
	// We have an SD card which supports SDR12 mode at 25MHz - Set frequency
	Emmc_switchClockRate(base_clock, SdClockNormal);
	
	wait(100); // Wait for clock rate to change

	if(card_supports_voltage_switch)
	{
		printf("ssed - Switching to 1.8V mode.\n");

		if(!Emmc_issueCommand(VoltageSwitch, 0))
		{
			printf("ssed - CMD[VoltageSwitch] failed.\n");

			// Power off
			gDevice.failed_voltage_switch = 1;

			Emmc_powerOff();

			return Emmc_Initialise();
		}

		// Disable SD clock
		gEmmc->Control1.raw &= ~(1 << 2);

		// Check DAT[3:0]
		unsigned int status_reg = gEmmc->Status.raw;
		unsigned int dat30 = (status_reg >> 20) & 0xF;
		if(dat30 != 0)
		{
			printf("ssed - DAT[3:0] did not settle to 0. Was: %d\n", dat30);
			
			gDevice.failed_voltage_switch = 1;

			Emmc_powerOff();

			return Emmc_Initialise();
		}

		// Set 1.8V signal enable to 1
		gEmmc->Control0.raw |= (1 << 8);

		wait(50);

		// Check to make sure signal enable is still set
		if(((gEmmc->Control0.raw >> 8) & 0x1) == 0)
		{
			printf("ssed - Controller did not keep the 1.8V signal high.\n");
			
			gDevice.failed_voltage_switch = 1;

			Emmc_powerOff();

			return Emmc_Initialise();
		}
	
		// Re enable sd clock
		gEmmc->Control1.raw |= (1 << 2);

		wait(10);

		status_reg = gEmmc->Status.raw;

		dat30 = (status_reg >> 20) & 0xF;

		if(dat30 != 0xF)
		{
            printf("ssed - DAT[3:0] did not settle to 1111b. Was: %d.\n", dat30);

			gDevice.failed_voltage_switch = 1;
			Emmc_powerOff();

			return Emmc_Initialise();
		}

		printf("ssed - Voltage switch complete.\n");
	}	

	if(!Emmc_issueCommand(AllSendCid, 0x0))
	{
		printf("ssed - Error sending ALL_SEND_CID.\n");
		return -1;
	}
	
	//printf("ssed - Got CID: %d %d %d %d\n", gDevice.last_resp3, gDevice.last_resp2, gDevice.last_resp1, gDevice.last_resp0);
	gDevice.cid[0] = gDevice.last_resp0;
	gDevice.cid[1] = gDevice.last_resp1;
	gDevice.cid[2] = gDevice.last_resp2;
	gDevice.cid[3] = gDevice.last_resp3;

	// Enter data state
	if(!Emmc_issueCommand(SendRelativeAddr, 0))
	{
		printf("ssed - Error sending SEND_RELATIVE_ADDR.\n");
		return -1;
	}

	unsigned int cmd3_resp = gDevice.last_resp0;

	//printf("ssed - Relative address: %d.\n", cmd3_resp);

	gDevice.rca = (cmd3_resp >> 16) & 0xFFFF;
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
	
	// Not select the card to toggle it to transfer state
	if(!Emmc_issueCommand(SelectCard, gDevice.rca << 16))
	{
		printf("ssed - Error sending SELECT_CARD.\n");
		return -1;
	}

	unsigned int cmd7_resp = gDevice.last_resp0;
	status = (cmd7_resp >> 9) & 0xF;

	if((status != 3) && (status != 4))
	{
		printf("ssed - Invalid status: %d.\n", status);
		return -1;
	}

	if(!gDevice.supports_sdhc)
	{
		if(!Emmc_issueCommand(SetBlockLen, 512))
		{
			printf("ssed - Error sending SET_BLOCKLEN.\n");
			return -1;
		}
	}

	// Black magic
	gDevice.block_size = 512;
	unsigned int controller_block_size = gEmmc->BlockCountSize.raw;
	controller_block_size &= (~0xFFF);
	controller_block_size |= 0x200;
	gEmmc->BlockCountSize.raw = controller_block_size;
		
	// This is a data command, so setup the buffer
	gDevice.receive_buffer = (unsigned int*)&(gDevice.scr.scr[0]); // 1174
	gDevice.block_size = 8;
	gDevice.blocks_to_transfer = 1;
	
	if(!Emmc_issueCommand(ACMD(51), 0))
	{
		printf("ssed -     Failed to send ACMD51.\n");
		return -1;
	}

	//printf("ssed - ACMD51 response: %d, %d, %d, %d\n", gDevice.last_resp0, gDevice.last_resp1, gDevice.last_resp2, gDevice.last_resp3);
		
	// Set back to default
	gDevice.block_size = 512;
	
	// ACMD51 is "kind" enough to give us the SCR in big endian... Swap it
	unsigned int scr0 = byte_swap(gDevice.scr.scr[0]);
	unsigned int sd_spec = (scr0 >> (56 - 32)) & 0xF;
	unsigned int sd_spec3 = (scr0 >> (47 - 32)) & 0x1;
	unsigned int sd_spec4 = (scr0 >> (42 - 32)) & 0x1;
	gDevice.scr.sd_bus_widths = (scr0 >> (48 - 32)) & 0xF;

	gDevice.scr.sd_version = SdVersionUnknown;
	if(sd_spec == 0)
		gDevice.scr.sd_version = SdVersion1;
	else if(sd_spec == 1)
		gDevice.scr.sd_version = SdVersion1_1;
	else if(sd_spec == 2)
	{
		if(sd_spec3 == 0)
			gDevice.scr.sd_version = SdVersion2;
		else if(sd_spec3 == 1)
		{
			if(sd_spec4 == 0)
				gDevice.scr.sd_version = SdVersion3;
			else if(sd_spec4 == 1)
				gDevice.scr.sd_version = SdVersion4;
		}
	}

	// 5 means both 4-bit and 8-bit are supported
	// It must support at least one, so fall back to 4-bit if 8 fails
	unsigned int data_mode = 8;
	if(gDevice.scr.sd_bus_widths != 5)
	{
		data_mode = 4;
		// Set 4-bit transfer mode (ACMD6)
		printf("ssed - Setting 4-bit data mode.\n");

		unsigned int old_interrupt_mask = gEmmc->IrptMask.raw;

		unsigned int new_interrupt_mask = old_interrupt_mask & ~(1 << 8);
		gEmmc->IrptMask.raw = new_interrupt_mask;

		if(Emmc_issueCommand(ACMD(6), 0x2))
		{
			printf("ssed - failed to switch to 4-bit data mode.\n");
		}
		else
		{
			gEmmc->Control0.raw |= 0x2;

			// Reenable card interrupt in host

			gEmmc->IrptMask.raw = old_interrupt_mask;

			printf("ssed - switch to 4-bit transfer mode complete.\n");
		}
	}

	
    printf("ssed - Found a valid %s (~%d~) SD card, using %d-bit transfer mode.\n", gSdVersionStrings[gDevice.scr.sd_version], gDevice.scr.sd_version, data_mode);

	gEmmc->Interrupt.raw = 0xFFFFFFFF;

	printf("ssed - Initialization complete, the SD card is now your bitch!\n");

	return 0;
}

int Emmc_powerOff(void)
{
	return Mailbox_SetDevicePowerState(HwId_Emmc, 0);
}

int Emmc_powerOn(void)
{
	return Mailbox_SetDevicePowerState(HwId_Emmc, 1);
}

unsigned int Emmc_getBaseClockHz()
{
    return Mailbox_SdGetBaseFrequency();
}

void Emmc_sdPowerOff(void)
{
	unsigned int control0 = gEmmc->Control0.raw;
	control0 &= ~(1 << 8);
	gEmmc->Control0.raw = control0;
}

int Emmc_powerCycle(void)
{
	printf("ssed - Power cycling:\n");
	
	unsigned int res = 0;
	if((res = Emmc_powerOff()) < 0)
	{
		return -1;
	}	
	
	wait(100);
	
	if((res = Emmc_powerOn()) < 0)
		printf("Failed!\n");
	else
		printf("Success!\n");
	
	return res;
}

//static int Emmc_ResetCommandLine()
//{
//	gEmmc->Control1.raw |= (1 << 25);
//
//	// Or are we waiting for it to be set to 0?
//	while((gEmmc->Control1.raw & (1 << 25)) == 1) { /* Do Nothing */ }
//
//	return 0;
//}

static int Emmc_resetDatLine()
{
	gEmmc->Control1.raw |= (1 << 26);

	while((gEmmc->Control1.raw & (1 << 26)) == 1) { /* Do Nothing */ } 

	return 0;
}

static void Emmc_handleCardInterrupt(void)
{
	unsigned int status = gEmmc->Status.raw;

	printf("ssed - card interrupt\n");
	printf("ssed - controller status: 0x%h", status);

	if(gDevice.rca)
	{
		if(!Emmc_issueCommand(SendStatus, gDevice.rca << 16))
		{
			printf("Failed to send SendStatus command when handling interrupt.\n");
		}
	}
}

static void Emmc_handleInterrupt()
{
	unsigned int irpt = gEmmc->Interrupt.raw;
	unsigned int reset_mask = 0;

	if(irpt & 1)
	{
		printf("ssed - Interrupt: Command complete.\n");
		reset_mask |= 1;
	}

	if(irpt & (1 << 1))
	{
		//printf("ssed - Interrupt: Card transfer complete.\n");
		reset_mask |= (1 << 1);
	}

	if(irpt & (1 << 2))
	{
		printf("ssed - Interrupt: Block gap event.\n");
		reset_mask |= (1 << 2);
	}

	if(irpt & (1 << 3))
	{
		printf("ssed - Interrupt: DMA.\n");
		reset_mask |= (1 << 3);
	}

	if(irpt & (1 << 4))
	{
		printf("ssed - Interrupt: Write buffer ready.\n");
		reset_mask |= (1 << 4);
	}

	if(irpt & (1 << 5))
	{
		printf("ssed - Interrupt: Read buffer ready.\n");
		reset_mask |= (1 << 5);
	}

	if(irpt & (1 << 6))
	{
		//printf("ssed - SD Card inserted.\n");
		reset_mask |= (1 << 6);
	}

	if(irpt & (1 << 7))
	{
		printf("ssed - SD Card removed.\n");
		reset_mask |= (1 << 7);
	}

	if(irpt & (1 << 8))
	{
		printf("ssed - Interrupt: Card interrupt.\n");
		volatile unsigned int wait = 0;
		while (wait < 2000000){
			wait++;
		}
		Emmc_handleCardInterrupt();
		reset_mask = (1 << 8);
	}

	if(irpt & 0x8000)
	{
		printf("ssed - Interrupt: Error occurred.\n");
		reset_mask |= 0xFFFF8000;
	}

	gEmmc->Interrupt.raw = reset_mask;
}

static int Emmc_ensureDataMode(void)
{
	if(gDevice.rca == 0)
	{
		int ret = Emmc_Initialise();

		if(ret != 0)
			return ret;
	}

	if(!Emmc_issueCommand(SendStatus, gDevice.rca << 16))
	{
		printf("ssed - EnsureDataMode() error sending CMD13.\n");
		gDevice.rca = 0;
		return -1;
	}

	unsigned int status = gDevice.last_resp0;
	unsigned int current_state = (status >> 9) & 0xF;

	if(current_state == 3)
	{
		// Currently in stand-by state - Select it
		if(!Emmc_issueCommand(SelectCard, gDevice.rca << 16))
		{
			printf("ssed - EnsureDataMode() no response from CMD17.\n");
			gDevice.rca = 0;
			return -1;
		}
	}
	else if(current_state == 5)
	{
		if(!Emmc_issueCommand(StopTransmitting, 0))
		{
			printf("ssed - EnsureDataMode() no response from CMD12.\n");
			gDevice.rca = 0;
			return -1;
		}

		Emmc_resetDatLine();
	}
	else if(current_state != 4)
	{
		// not in transfer state - Re initialize
		int ret = Emmc_Initialise();
		if(ret != 0)
			return ret;
	}

	if(current_state != 4)
	{
		printf("ssed - EnsureDataMode() rechecking status: \n");

		if(Emmc_issueCommand(SendStatus, gDevice.rca << 16))
		{
			printf("ssed - EnsureDataMode() no response from CMD13\n");
			gDevice.rca = 0;
			return -1;
		}

		status = gDevice.last_resp0;
		current_state = (status >> 9) & 0xF;

		printf("ssed - current state: %d\n", current_state);

		if(current_state != 4)
		{
			printf("ssed - Unable to initialize SD card to Data mode (state %d)\n", current_state);
			gDevice.rca = 0;
			return -1;
		}
	}

	return 0;
}

int Emmc_issueCommandInt(unsigned int command, unsigned int argument)
{
	// Check command inhibit
	while(gEmmc->Status.raw & 0x1)
		wait(5);

	// With busy command
	if((command & SD_CMD_RSPNS_TYPE_MASK) == SD_CMD_RSPNS_TYPE_48B)
	{
		if((command & SD_CMD_RSPNS_TYPE_MASK) != SD_CMD_TYPE_ABORT)
		{
			// Not abort command
			while((gEmmc->Status.raw & 0x2))
				wait(10);
		}
	}

	unsigned int is_sdma = 0;
	if((command & SD_CMD_ISDATA) && gEmmcUseDMA)
	{
		printf("ssed - Performing SDMA transfer, current interrupt: %d.\n", gEmmc->Interrupt.raw);

		is_sdma = 1;
	}

	if(is_sdma)
		gEmmc->Arg2 = SDMA_BUFFER_PA;

	if(gDevice.blocks_to_transfer > 0xFFFF)
	{
		printf("ssed - blocks to transfer too great: %d.\n", gDevice.blocks_to_transfer);
		return -1;
	}

	unsigned int blksizecnt = gDevice.block_size | (gDevice.blocks_to_transfer << 16);
	gEmmc->BlockCountSize.raw = blksizecnt;

	// Set argument
	gEmmc->Arg1 = argument;

	if(is_sdma)
		command |= SD_CMD_DMA;

	gEmmc->Cmdtm.raw = command;

	wait(5);
	
	while(gEmmc->Interrupt.bits.cmd_done == 0 && gEmmc->Interrupt.bits.err == 0) { /* Do Nothing  */} 

	unsigned int irpts = gEmmc->Interrupt.raw;

	gEmmc->Interrupt.raw = 0xFFFF0001;

	if((irpts & 0xFFFF0001) != 0x1)
	{
		printf("ssed - Error occurred whilst waiting for command complete interrupt.\n");

		gLastError = irpts & 0xFFFF0000;
		return -1;
	}

	wait(100);

	// Get response data
	switch(command & SD_CMD_RSPNS_TYPE_MASK)
	{
	case SD_CMD_RSPNS_TYPE_48:
	case SD_CMD_RSPNS_TYPE_48B:
		gDevice.last_resp0 = gEmmc->Resp0;
		break;
	case SD_CMD_RSPNS_TYPE_136:
		gDevice.last_resp0 = gEmmc->Resp0;
		gDevice.last_resp1 = gEmmc->Resp1;
		gDevice.last_resp2 = gEmmc->Resp2;
		gDevice.last_resp3 = gEmmc->Resp3;
		break;
	}

	if((command & SD_CMD_ISDATA) && (is_sdma == 0))
	{
		unsigned int wr_irpt;
		int is_write = 0;

		if(command & SD_CMD_DAT_DIR_CH)
			wr_irpt = (1 << 5);  // Read
		else
		{
			is_write = 1;
			wr_irpt = (1 << 4);  // Write
		}

		int current_block = 0;
		unsigned int* current_buffer_address = gDevice.receive_buffer;

		while(current_block < gDevice.blocks_to_transfer)
		{
			if(gDevice.blocks_to_transfer > 1)
				printf("ssed - Multi block transfer, awaiting block %d ready.\n", current_block);

			while(!(gEmmc->Interrupt.raw & (wr_irpt | 0x8000))) { /* Do Nothing */ }

			irpts = gEmmc->Interrupt.raw;

			gEmmc->Interrupt.raw = 0xFFFF0000 | wr_irpt;

			if((irpts & (0xFFFF0000 | wr_irpt)) != wr_irpt)
			{
				gLastError = irpts & 0xFFFF0000;
				return -1;
			}

			// Transfer block
			unsigned int current_byte_number = 0;
			while(current_byte_number < gDevice.block_size)
			{
				if(is_write)
				{
					unsigned int data = read_word((unsigned char*)current_buffer_address, 0);
					gEmmc->Data = data;
				}
				else
				{
					unsigned int data = gEmmc->Data;
					write_word(data, (unsigned char*)current_buffer_address, 0);
				}

				current_byte_number += 4;
				current_buffer_address++;
			}

			current_block++;
		}
	}

	if((((command &  SD_CMD_RSPNS_TYPE_MASK) == SD_CMD_RSPNS_TYPE_48B) || 
		(command & SD_CMD_ISDATA)) && (is_sdma))
	{
		if((gEmmc->Status.raw & 0x2) == 0)
			gEmmc->Interrupt.raw = 0xFFFF0002;
		else
		{
			while(!(gEmmc->Interrupt.raw & 0x8002)) { /* Do Nothing */ }

			irpts = gEmmc->Interrupt.raw;

			gEmmc->Interrupt.raw = 0xFFFF0002;

			if(((irpts & 0xFFFF0002) != 0x2) && ((irpts & 0xFFFF0002) != 0x100002))
			{
				printf("ssed - Error occurred whilst waiting for transfer complete interrupt.\n");

				gLastError = irpts & 0xFFFF0000;
				return -1;
			}

			gEmmc->Interrupt.raw = 0xFFFF0002;
		}
	}
	else if(is_sdma)
	{
		// We have to wait for either transfer complete, DMA interrupt or an error

		// First make DAT is not already 0
		if((gEmmc->Status.raw & 0x2) == 0)
			gEmmc->Interrupt.raw = 0xFFFF000A;
		else
		{
			while(!(gEmmc->Interrupt.raw & 0x800A)) { /* Do Nothing */ }

			irpts = gEmmc->Interrupt.raw;

			gEmmc->Interrupt.raw = 0xFFFF000A;

			// Detect errors
			if((irpts & 0x8000) && ((irpts & 0x2) != 0x2))
			{
				printf("ssed - error occurred whilst waiting for transfer complete interrupt.\n");
				gLastError = irpts & 0xFFFF0000;
				return -1;
			}

			if((irpts & 0x8) && ((irpts & 0x2) != 0x2))
			{
				printf("ssed - Error: DMA interrupt occurred without transfer complete interrupt.\n");
				gLastError = irpts & 0xFFFF0000;
				return -1;
			}

			// Transfer complete?
			if(irpts & 0x2)
			{
				printf("ssed - SDMA transfer complete.\n");

				my_memcpy(gDevice.receive_buffer, (const void*)SDMA_BUFFER, gDevice.block_size);
			}
			else
			{
				// Unknown error
				if(irpts == 0)
					printf("ssed - timeout waiting for SDMA transfer to complete.\n");
				else
					printf("ssed - Unknown SDMA transfer error\n");

				printf("ssed - Interrupt: %h, Status: %h.\n", irpts, gEmmc->Status.raw);

				if((irpts == 0) && ((gEmmc->Status.raw & 0x3) == 0x2))
				{
					// The data transfer is still going - try to stop it
					printf("ssed - Aborting transfer.\n");

					gEmmc->Cmdtm.raw = CMD[StopTransmitting];

					wait(2000);
				}
				gLastError = irpts & 0xFFFF0000;
				return -1;
			}
		}
	}

	return 1;
}

int Emmc_issueCommand(unsigned int command, unsigned int argument)
{
	Emmc_handleInterrupt();

	if(command & IS_APP_CMD)
	{
		command &= 0xFF;
		if(ACMD[command] == SD_CMD_RESERVED(0))
		{
			printf("ssed - Invalid ACMD%d\n", command);
			return -1;
		}

		gLastCommand = AppCmd;

		unsigned int rca = 0;
		if(gDevice.rca)
			rca = gDevice.rca << 16;

		if(Emmc_issueCommandInt(CMD[AppCmd], rca))
		{
			//printf("ssed - APP_CMD Sent, sending ACMD%d.\n", command);

			gLastCommand = command | IS_APP_CMD;

			Emmc_issueCommandInt(ACMD[command], argument);
		}
	}
	else
	{
		//printf("ssed - Issuing CMD%d.\n", command);
		if(CMD[command] == SD_CMD_RESERVED(0))
		{
			printf("ssed - Invalid command CMD%d.\n", command);
			return -1;
		}

		gLastCommand = command;

		Emmc_issueCommandInt(CMD[command], argument);
	}

	return 1;
}

unsigned int Emmc_doDataCommand(char* buf, unsigned int is_write, unsigned int buflen, unsigned int block_number)
{
	if(!gDevice.supports_sdhc)
		block_number *= 512;

	if(buflen < gDevice.block_size)
	{
		printf("Buffer too small to read an entire block.\n");
		return -1;
	}

	gDevice.blocks_to_transfer = buflen / gDevice.block_size;

	if(buflen % gDevice.block_size)
	{
		printf("Called with a buffer size (%d) that is not a multiple of the block size (%d).\n", buflen, gDevice.block_size);
		return -1;
	}

	gDevice.receive_buffer = (unsigned int*)buf;

	int command;
	if(is_write)
	{
		if(gDevice.blocks_to_transfer > 1)
			command = WriteMultipleBlocks;
		else
			command = WriteBlock;
	}
	else
	{
		if(gDevice.blocks_to_transfer > 1)
			command = ReadMultipleBlocks;
		else
			command = ReadSingleBlock;
	}

	int retry_count = 0;
	int max_retries = 3;
	while(retry_count < max_retries)
	{
		if(Emmc_issueCommand(command, block_number))
			break;
		else
		{
			printf("ssed - Error sending command CMD%d, error: %d\n", command, gLastError);
			retry_count++;
			if(retry_count < max_retries)
				printf("ssed - Retrying...\n");
			else
				printf("ssed - Giving up...\n");
		}
	}

	if(retry_count == max_retries)
	{
		gDevice.rca = 0;
		return -1;
	}

	return 0;
}

int Emmc_ReadBlock(char* buf, unsigned int buflen, unsigned int block_number)
{
	if(Emmc_ensureDataMode() != 0)
		return -1;

	if(Emmc_doDataCommand(buf, 0, buflen, block_number) < 0)
		return -1;

	return buflen;
}

//static int Emmc_SuitableForDma(void *buf)
//{
//	if((unsigned int)buf & 0xfff)
//        return 0;
//    else
//        return 1;
//}
