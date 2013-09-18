#define EMMC_BASE 0x20300000

// SdClockSpeed - SD Clock Frequencies (in Hz)
typedef enum {
	SdClockId       = 400000,
	SdClockNormal   = 25000000,
	SdClockHigh     = 50000000,
	SdClock100      = 100000000,
	SdClock208      = 208000000
} SdClockSpeed;

// emmc_status_register
typedef union{
	unsigned int raw;
	struct{
		volatile unsigned int CmdInhibit : 1;
		volatile unsigned int DatInhibit : 1;
		volatile unsigned int DatActive : 1;
		volatile unsigned int reserved : 5;
		volatile unsigned int WriteTransfer : 1;
		volatile unsigned int ReadTransfer : 1;
		volatile unsigned int reserved2 : 10;
		volatile unsigned int DatLevel0 : 4;
		volatile unsigned int CmdLevel : 1;
		volatile unsigned int DatLevel1 : 4;
		volatile unsigned int reserved3 : 3;
	} bits;
} emmc_status_register;

// blksizecnt_register
typedef union {
	unsigned int raw;
	struct {
		volatile unsigned int BlkSize : 10;
		volatile unsigned int reserved : 6;
		volatile unsigned int BlkCnt : 16;
	} bits;
} blksizecnt_register;

// tm_auto_cmd_en
typedef enum {
	NoCommand = 0,
	Command12 = 1,
	Command23 = 2,
	reserved = 3
} tm_auto_cmd_en;

// cmd_rspns_type
typedef enum {
	ResponseNone = 0,
	Response136Bits = 1,
	Response48Bits = 2,
	Response48BitsBusy = 3
} cmd_rspns_type;

// cmd_type
typedef enum {
	Normal = 0,
	Suspend = 1, // Suspend current data transfer
	Resume = 2,  // Resume last data transfer
	Abort = 3    // Abort the current data transfer
} cmd_type;

// cmdtm_register
typedef union {
	unsigned int raw;
	struct {
		volatile unsigned int reserved : 1;
		volatile unsigned int TmBlkCntEn : 1;   // Enable block counter for multiple block transfers (1 = eanble)
		tm_auto_cmd_en TmAutoCmdEn : 2;         // Select command to send after completion of data transfer
		volatile unsigned int TmDatDir : 1;     // Direction of data transfer (0 = host -> Card, 1 = card -> host)
		volatile unsigned int TmMultiBlock : 1; // Type of transfer (0 = single, 1 = multi)
		volatile unsigned int reserved2 : 10;   // -
		cmd_rspns_type CmdRspnsType : 2;        // Type of the response (see cmd_rspns_type)
		volatile unsigned int CmdCrcChkEn : 1;  // Check the responses CRC (1 = enable)
		volatile unsigned int CmdIxchkEn : 1;   // Check index has same index as command 
		volatile unsigned int CmdIsData : 1;    // 0 = no data transfer command
		cmd_type CmdType : 2;                   // The type of the command to be issued
		volatile unsigned int CmdIndex : 6;     // Index of command to be issued
		volatile unsigned int reserved3 : 2;    // -
	} bits;
} cmdtm_register;

// Emmc
typedef struct { // Placed at EMMC_BASE
	volatile unsigned int Arg2;            // ACMD23 Argument
	blksizecnt_register BlockCountSize;    // Block size and count
	volatile unsigned int Arg1;            // Argument
	cmdtm_register Cmdtm;                  // Command and transfer mode
	volatile unsigned int Resp0;           // Response bits 31 : 0
	volatile unsigned int Resp1;           // Response bits 63 : 32
	volatile unsigned int Resp3;           // Response bits 95 : 64
	volatile unsigned int Resp4;           // Response bits 127 : 96
	volatile unsigned int Data;            // Data
	emmc_status_register Status;           // Status
	volatile unsigned int Control0;        // Host configuration 0
	volatile unsigned int Control1;        // Host configuration 1
	volatile unsigned int Interrupt;       //    Interrupt Flags
	volatile unsigned int IrptMask;        //    Interrupt flag enable
	volatile unsigned int IrptEn;          //    Interrupt Generation Enable
	volatile unsigned int Control2;        // Host configuration 2
	volatile unsigned int ForceIrpt;       // Force interupt event
	volatile unsigned int BootTimeout;     // Timeout in boot mode
	volatile unsigned int DbgSel;          // Debug bus configuration
	volatile unsigned int ExrdfifoCfg;     // Extension FIFO configuration
	volatile unsigned int ExrdfifoEnable;  // Extension FIFO enable
	volatile unsigned int TuneStep;        // Delay per card clock turning step
	volatile unsigned int TuneStepsStd;    // Card clock tuning steps for SDR
	volatile unsigned int TuneStepsDdr;    // Card clock tuning steps for DDR
	volatile unsigned int SpiIntSpt;       // SPI Interrupt support
	volatile unsigned int SlotisrVer;      // Slot interrupt status and version
} Emmc;

// EmmcCommand
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

// SdCommand
typedef struct {
	unsigned char start:1;
	unsigned char transmission:1; // direction, 1 = host, 0 = card
	unsigned char commandIndex:6;
	unsigned int argument:32;
	unsigned int crc7:7;
	unsigned int end:1;
} SdCommand;

// SdCardStatus
typedef struct{
	unsigned char out_of_range : 1;
	unsigned char address_error : 1;
	unsigned char block_len_error: 1;
	unsigned char erase_seq_error : 1;
	unsigned char erase_param : 1;
	unsigned char wp_violation : 1;	
	unsigned char card_is_locked : 1;
	unsigned char lock_unlocked_failed : 1;
	unsigned char com_crc_error : 1;
	unsigned char illegal_command : 1;
	unsigned char card_ecc_failed : 1;
	unsigned char cc_error : 1;
	unsigned char error : 1;
	unsigned char reserved1 : 1;
	unsigned char reserved2 : 1;
	
	unsigned char csd_overwrite : 1;
	unsigned char wp_erase_skip : 1;
	unsigned char card_ecc_disabled : 1;	
	unsigned char erase_reset : 1;
	unsigned char current_state : 4;
	unsigned char ready_for_data : 1;
	unsigned char reserved3 : 1;
	unsigned char app_cmd : 1;
	unsigned char reserved4 : 1;
	
	unsigned char ake_seq_error : 1;
} SdCardStatus;

// SdResponse1 (48-Bit)
typedef struct { // 48-Bit
	unsigned char start:1;
	unsigned char transmission:1;
	unsigned char commandIndex:6;
	unsigned int cardStatus:32;
	unsigned char crc7:7;
	unsigned char end:1
} SdResponse1;

// SdResponse2 (136-Bit)
typedef struct { // 136-bit
	unsigned char start:1;
	unsigned char transmission:1;
	unsigned char reserved:6;
	unsigned int Cid[4];
	unsigned char end:1
} SdResponse2;

// SdResponse2 (48-Bit)
typedef struct { // 48-bit
	unsigned char start:1;
	unsigned char transmission:1;
	unsigned char reserved:6;
	unsigned int Ocr:32;
	unsigned char reserved2: 8;
	unsigned char end:1;
} SdResponse3;

// SdResponse6 (48-Bit)
typedef struct { // 48-Bit
	unsigned char start:1;
	unsigned char transmission:1;
	unsigned char commandIndex:6;
	unsigned int NewPublishedRca:16;
	unsigned int CardStatus:16;
	unsigned int Crc7:7;
	unsigned int end:1
} SdResponse6;

/* R7
// Voltage accepted  Value Definition
// 0000b  Not Defined
// 0001b  2.7-3.6V
// 0010b  Reserved for Low Voltage Range
// 0100b  Reserved
// 1000b  Reserved
  Others  Not Defined */
// SdResponse7 (48-Bit)
typedef struct { // 48-Bit
	unsigned char start:1;
	unsigned char tranmission:1;
	unsigned char commandIndex:6;
	unsigned int reserved:20;
	unsigned char voltageAccepted:4;
	unsigned char checkPatternEcho:8;
	unsigned char crc7:7;
	unsigned char end:1
} SdResponse7;

unsigned int EmmcInitialise(void);
unsigned int EmmcGetClockSpeed(void);
unsigned int EmmcPowerOn(void);
unsigned int EmmcPowerOff(void);
unsigned int EmmcPowerCycle(void);
unsigned int EmmcSetClockRate(unsigned int clock, unsigned int targetRate);
unsigned int EmmcSendCommand(EmmcCommand cmd, unsigned int argument, unsigned int timeout);
unsigned int EmmcRead(unsigned char* buf, unsigned int bufLen, unsigned int blockToReadFrom);
unsigned int EmmcWrite(unsigned char* buf, unsigned int bufLEn, unsigned int blockToWriteTo);
