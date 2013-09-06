#include "emmc.h"

Emmc* gEmmc;

unsigned int EmmcInitialise(void)
{
	gEmmc = (Emmc*)EMMC_BASE;
	
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
}