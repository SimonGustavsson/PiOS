#define EMMC_BASE 0x7E300000

// SD Clock Frequencies (in Hz)
typedef enum {
	SdClockId       = 400000,
	SdClockNormal   = 25000000,
	SdClockHigh     = 50000000,
	SdClock100      = 100000000,
	SdClock208      = 208000000
} SdClockSpeed;

typedef struct { // Placed at EMMC_BASE
	unsigned int Arg2;
	unsigned int BlockCountSize;
	unsigned int Arg1;
	unsigned int Cmdtm;
	unsigned int Resp0;
	unsigned int Resp1;
	unsigned int Resp3;
	unsigned int Data;
	unsigned int Status;
	unsigned int Control0;
	unsigned int Control1;
	unsigned int Interrupt;
	unsigned int IrptMask;
	unsigned int Control2;
	unsigned int ForceIrpt;
	unsigned int BootTimeout;
	unsigned int DbgSel;
	unsigned int ExrdfifoCfg;
	unsigned int ExrdfifoEnable;
	unsigned int TuneStep;
	unsigned int TuneStepsStd;
	unsigned int TuneStepsDdr;
	unsigned int SpiIntSpt;
	unsigned int SlotisrVer;
} Emmc;

typedef enum {
	GoIdleState          = 0,	// Resets SD card
	SendOpCond           = 1,	// Sends host capacity support information and kick off card initialization
	AllSendCid           = 2,	// Broadcast to all commands to send cid
	SendRelativeAddr     = 3,	// Request new relative card adress (RCA) from card
	
	SetDsr               = 4,	// Programs the DSR of all cards
	IOSetOpCond          = 5,
	SwitchFunction       = 6,	// Checks switchable function (mode 0) and switch card function
	SelectCard           = 7,	// Select card
	DeselectCard         = 7,	// Deselect card
	SelectDetectCard     = 7,	// Toggles card selection
	SendIfCond           = 8,	// Sends SD card interface condition, including voltage and asks whether card supports it
	
	SendCsd 			 = 9, 	// Requests card specific CSD on the CMD line
	SendCid              = 10,  // Requests card specific card identification (CID) on the CMD line
	VoltageSwitch        = 11,	// Switch to 1.8V bus signaling level
	StopTransmitting     = 12,	// Forces the card to stop the current transmission
	SendStatus           = 13,  // Requests card specfic status register
	
	GoInactiveState      = 15,	// Sends a card into inactive state, used to deactivate a card
	SetBlockLen          = 16,	// Sets block length for Read/Write/Lock, default block length is 512 Bytes
	ReadSingleBlock      = 17,	// Reads a single block of the size set by the SET_BLOCKLEN command
	ReadMultipleBlocks   = 18,	// Continuously transfers blocks from card to host until STOP_TRANSMISSION is received, block length is same as READ_SINGLE_BLOCK
	SendTuningBlock      = 19,	// 64-bytes tuning pattern is sent for SDR50 and SDR104
	SpeedClassControl    = 20,	// Speed class control command (for details see 4.13.2.8 in SD Physical Layer Simplified Specification.pdf)
	
	SetBlockCount        = 23,	// Sets block count for READ_SINGLE_BLOCK and WRITE_MULTIPLE_BLOCK
	WriteBlock           = 24,	// For SDSC the length is set by SET_BLOCKLEN, for SDHC & SDXC block length is a fixed 512 Bytes regardless
	WriteMultipleBlocks  = 25,	// Continuously writes blocks of data until a STOP_TRANSMISSION is received, block length is same as WRITE_BLOCK
	
	ProgramCsd			 = 27,	// Programs the programmable bits of the CSD register
	SetWriteProt         = 28,	// If card supports write protection, this command sets the protection bit on the addressed group (NOT supported by SDHC/SDXC)
	ClrWriteProt         = 29,	// If card supports write protection, this command clears the protection bit (NOT supported by SDHC/SDXC)
	SendWritePRot        = 30,	// If card supports write protection, this command requests the status of the protection bit (NOT supported by SDHC/SDXC)
	
	EraseWrBlkStart      = 32,	// Sets the address of the first write block to be erased
	EraseWrBlkEnd        = 33,	// Sets the address of the last write block of the continuous range to be erased
	Erase                = 38,	// Erases all previously selected write blocks
	LockUnlock           = 42,	// Used to set/reset the password or lock/unlock the card.
	AppCmd               = 55,	// Defines to the card that the next command is an application specific command
	GenCmd               = 56	// Used either to read or write a data block from the card for general purpose/application specific commands.
} EmmcCommand;

unsigned int EmmcInitialise(void);
unsigned int EmmcGetClockSpeed(void);
unsigned int EmmcPowerOn(void);
unsigned int EmmcPowerOff(void);
unsigned int EmmcPowerCycle(void);
unsigned int EmmcSetClockRate(unsigned int clock, unsigned int targetRate);
unsigned int EmmcSendCommand(EmmcCommand cmd, unsigned int argument, unsigned int timeout);
unsigned int EmmcRead(unsigned char* buf, unsigned int bufLen, unsigned int blockToReadFrom);
unsigned int EmmcWrite(unsigned char* buf, unsigned int bufLEn, unsigned int blockToWriteTo);
