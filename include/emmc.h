#define EMMC_BASE 0x20300000

// SD Clock Frequencies (in Hz)
typedef enum {
	SdClockId       = 400000,
	SdClockNormal   = 25000000,
	SdClockHigh     = 50000000,
	SdClock100      = 100000000,
	SdClock208      = 208000000
} SdClockSpeed;

typedef struct { // Placed at EMMC_BASE
	volatile unsigned int Arg2;
	volatile unsigned int BlockCountSize;
	volatile unsigned int Arg1;
	volatile unsigned int Cmdtm;
	volatile unsigned int Resp0;
	volatile unsigned int Resp1;
	volatile unsigned int Resp3;
	volatile unsigned int Data;
	volatile unsigned int Status;
	volatile unsigned int Control0;
	volatile unsigned int Control1;
	volatile unsigned int Interrupt;
	volatile unsigned int IrptMask;
	volatile unsigned int Control2;
	volatile unsigned int ForceIrpt;
	volatile unsigned int BootTimeout;
	volatile unsigned int DbgSel;
	volatile unsigned int ExrdfifoCfg;
	volatile unsigned int ExrdfifoEnable;
	volatile unsigned int TuneStep;
	volatile unsigned int TuneStepsStd;
	volatile unsigned int TuneStepsDdr;
	volatile unsigned int SpiIntSpt;
	volatile unsigned int SlotisrVer;
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

#define CONTROL0_ALT_BOOT_EN   (1 << 22)  // Enable alternative boot mode access
#define CONTROL0_BOOT_EN       (1 << 21)  // Boot mode access
#define CONTROL0_SPI_MODE      (1 << 20)  // SPI Mode enable
#define CONTROL0_GAP_IEN       (1 << 19)  // Enable SDIO interrupt
#define CONTROL0_READWAIT_EN   (1 << 18)  // Use DATA2 read-wait protocol
#define CONTROL0_GAP_RESTART   (1 << 17)  // Restart a transaction that was stopped using GAP_STOP
#define CONTROL0_GAP_STOP      (1 << 16)  // Stop the current transaction
#define CONTROL0_HCTL_8BIT     (1 << 5)   // Use 8 Data lines
#define CONTROL0_HCTL_HS_EN    (1 << 2)   // Select high-speed mode
#define CONTROL0_HCTL_HS_DWITH (1 << 1)   // Use 4 data lines

#define CONTROL1_SRST_DATA      (1 << 26) // Reset data handling
#define CONTROL1_SRST_CMD       (1 << 25) // Reset command handling
#define CONTROL1_SRST_HC        (1 << 24) // Reset host circuit
#define CONTROL1_DATA_TOUNIT(x) (x << 19) // Data timeout unit exp
#define CONTROL1_CLK_FREQ8(x)   (x << 15) // SD Clock base divier LSB
#define CONTROL1_CLK_FREQ_MS2(x)(x << 7)  // SD clock base divier MSB 
#define CONTROL1_CLK_GENSEL     (1 << 5)  // Clock gen mode
#define CONTROL1_CLK_EN         (1 << 2)  // SD clock enable
#define CONTROL1_CLK_STABLE     (1 << 1)  // Sd clock stable 
#define CONTROL1_CLK_INTLEN     (1 << 0)  // Internal EMMC clock enable

#define RESPONSE_R7_VOLTAGE_NOT_DEFINED  (1 << 16)
#define RESPONSE_R7_VOLTAGE_NORMAL       (1 << 17)
#define RESPONSE_R7_VOLTAGE_LOW_RESERVED (1 << 18)

#define COMMAND_TRANSMISSION_BIT (1 << 46)
#define COMMAND_COMMAND_INDEX(x) (x << 45)
#define COMMAND_ARGUMENT(x) (x << 39)

typedef struct {
	unsigned char start:1;
	unsigned char transmission:1; // direction, 1 = host, 0 = card
	unsigned char commandIndex:6;
	unsigned int argument:32;
	unsigned int crc7:7;
	unsigned int end:1;
} SdCommand;

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
/* Card status:
	
*/
typedef struct { // 48-Bit
	unsigned char start:1;
	unsigned char transmission:1;
	unsigned char commandIndex:6;
	unsigned int cardStatus:32;
	unsigned char crc7:7;
	unsigned char end:1
} SdResponse1;

typedef struct { // 136-bit
	unsigned char start:1;
	unsigned char transmission:1;
	unsigned char reserved:6;
	unsigned int Cid[4];
	unsigned char end:1
} SdResponse2;

typedef struct { // 48-bit
	unsigned char start:1;
	unsigned char transmission:1;
	unsigned char reserved:6;
	unsigned int Ocr:32;
	unsigned char reserved2: 8;
	unsigned char end:1;
} SdResponse3;

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
