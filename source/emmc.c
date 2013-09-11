#include "emmc.h"
#include "mailbox.h"
#include "timer.h"
#include "stringutil.h"

volatile Emmc* gEmmc;

unsigned int EmmcInitialise(void)
{
	gEmmc = (Emmc*)EMMC_BASE;
	
	unsigned int ver = gEmmc->SlotisrVer;
	unsigned int vendor = ver >> 24;
	unsigned int sdversion = (ver >> 16) & 0xff;
	unsigned int slot_status = ver & 0xff;
	
	printf("EMMC: vendor %d, sdversion %d, slot status %d\n", vendor, sdversion, slot_status);
		
	return 0; // Code below not finished
	
	
	// Reset the entire circuit
	gEmmc->Control1 |= (0x1 << 24); 
	
	printf("Control1 = '%d'", gEmmc->Control1);
	
	// Wait for reset
	while((gEmmc->Control1 & (0x7 << 24)) != 0);
	
	printf("ssed - Control1 indicates success resetting host");
	
	// Set clock speed
	gEmmc->Control1 |= 0x8001;
	
	// Wait for clock speed to set
	while(!(gEmmc->Control1 & 0x2));
	
	// Confirm status
	if((gEmmc->Status & (0x1 << 16)))
	{
		// Success!?
		printf("ssed - Status bit 16 is set woho!");
	}
	
	return 0;
	
	// Set IRPT_EN to 0? 
	// Set IRT_MASK
	// Set INTERRUPT
	
	// Send CMD0
	// Wait for flag in Interrupt
	
	return 0;
	
	
	
	// TODO: Flesh out pseudo code
	// Send CMD0
	
	// send CMD8
	//		response: Version 2 > SD
	//	  		valid response
	//				Non-compatible voltage range or check pattern is not correct UNUSABLE CARD
	//				
	//			Card with compatible voltage range
	// 				ACMD41 with HCS=0 or 1 (1 if we support high capacity)
	//					Card ready?
	//						Yes - is CCS in response?
	//							CCS 0: Ver 2.0 > SD card
	//							CCS 1: Ver 2.0 > SDHC or SDXC
	//						No - Send ACM41 again
	//						Timed out - Unusable card
	// 		no response : return
	//		
	//		Set voltage to High 
	// 		Request card id
	// 		Request card address
	// 		Place card in transfer state
	
	return 0;
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
	printf("ssed - Power cycling SD-card.\n");
	if(EmmcPowerOff() < 0)
		return -1;
		
	wait(50);
	
	return EmmcPowerOn();
}
