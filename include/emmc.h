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
		volatile unsigned int CmdInhibit : 1;     // Command line still in use (1 = yes)
		volatile unsigned int DatInhibit : 1;     // Data line still in use (1 = yes)
		volatile unsigned int DatActive : 1;      // At least one data line is active (1 = yes)
		volatile unsigned int reserved : 5;       // -
		volatile unsigned int WriteTransfer : 1;  // New data can be written to the EMMC
		volatile unsigned int ReadTransfer : 1;   // New data can be read from EMMC
		volatile unsigned int reserved2 : 10;     // -
		volatile unsigned int DatLevel0 : 4;      // Values of DAT3 to DAT0
		volatile unsigned int CmdLevel : 1;       // Value of command line CMD
		volatile unsigned int DatLevel1 : 4;      // Value of data lines DAT7 to DAT4
		volatile unsigned int reserved3 : 3;      // -
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

// control0_register
typedef union {
	unsigned int raw;
	struct {
		volatile unsigned int hctl_dwidth : 1; // Use 4 data lines (1 = enabled)
		volatile unsigned int hctl_hs_en : 1;  // Select high speed mode (i.e. DAT and CMD lines change on the rising CLK edge) (1 = enabled)
		volatile unsigned int reserved : 2;    // -
		volatile unsigned int hctl_8bit : 1;   // Use 8 data lines (1 = enabled)
		volatile unsigned int reserved2 : 11;  // -
		volatile unsigned int gap_stop : 1;    // Stop the current transaction at the next block gap (0 = ignore, 1 = stop)
		volatile unsigned int gap_restart : 1; // Restart a transaction which was stopped using the gap_stop bit (0 = ignore, 1 = restart)
		volatile unsigned int readwait_en : 1; // Use DAT2 read-wait protocol for SDIO cards supporting this (1 = enabled)
		volatile unsigned int gap_ien : 1;     // Enable SDIO interrupt at block gap (only if the hctl_dwidth bit is set) (1 = enabled)
		volatile unsigned int spi_mode : 1;    // SPI mode enable (0 = normal, 1 = SPI)
		volatile unsigned int boot_en : 1;     // boot mode access (0 = stop boot mode access, 1 = start boot mode access)
		volatile unsigned int alt_boot_en : 1; // Enable alternative boot mode access (1 = enabled)
		volatile unsigned int reserved3 : 9;   // -
	} bits;
} control0_register;

// control1_register
typedef union {
	unsigned int raw;
	struct {
		volatile unsigned int clk_intlen : 1;  // Clock enablefor internal emmc clocks for power saving (1 = enabled)
		volatile unsigned int clk_stable : 1;  // SD clock stable (0 = no)
		volatile unsigned int clk_en : 1;      // SD clock enable (1 = enabled)
		volatile unsigned int reserved : 2;    // -
		volatile unsigned int clk_gensel : 1;  // Mode of clock generation (0 = divided, 1 = programmable)
		volatile unsigned int clk_freq_ms2 : 2;// SD clock base divider MSBs
		volatile unsigned int clk_freq8 : 8;   // SD clock base divider LSBs
		volatile unsigned int data_taunit : 4; // Data timeout unit exponent (1111 = disabled)
		volatile unsigned int reserved2 : 4;   // -
		volatile unsigned int srst_hc : 1;     // Reset the complete host circuit (1 = enabled)
		volatile unsigned int srst_cmd : 1;    // Reset the command handler circuit (1 = enabled)
		volatile unsigned int srst_data : 1;   // Reset the data handling circuit (1 = enabled)
		volatile unsigned int reserved3 : 5;   // -
	} bits;
} control1_register;

// interrupt_register - Holds the interrupt flags, each flag can be disabled in IRPT_MASK
typedef union {
	unsigned int raw;
	struct {
		volatile unsigned int cmd_done : 1;   // Commnad has finished (1 = yes)
		volatile unsigned int data_done : 1;  // Data transfer has finished (1 = yes)
		volatile unsigned int block_gap : 1;  // Data transfer has stopped at block gap (1 = yes)
		volatile unsigned int reserved : 1;   // -
		volatile unsigned int write_rdy : 1;  // Data can be written to DATA register (1 = yes)
		volatile unsigned int read_rdy : 1;   // Data register contains dta to be read (1 = yes)
		volatile unsigned int reserved2 : 2;  // -
		volatile unsigned int card : 1;       // Card made interrupt request (1 = yes)
		volatile unsigned int reserved3 : 3;  // -
		volatile unsigned int retune : 1;     // Clock retune request was made (1 = yes)
		volatile unsigned int bootack : 1;    // Boot acknowledge has received (1 = yes)
		volatile unsigned int endboot : 1;    // boot operation has terminated (1 = yes)
		volatile unsigned int err : 1;        // An error has occured (1 = error)
		volatile unsigned int cto_err : 1;    // Timeout on command line (1 = error)
		volatile unsigned int ccrc_err : 1;   // Command CRC error (1 = error)
		volatile unsigned int cend_err : 1;   // End bit on command line not 1 (1 = error)
		volatile unsigned int cbad_err : 1;   // Incorrect command index in response (1 = error)
		volatile unsigned int dto_err : 1;    // Timeout on data line (1 = error)
		volatile unsigned int dcrc_err : 1;   // Data CRC error (1 = error)
		volatile unsigned int dend_error : 1; // End bit on data line not 1 (1 = error)
		volatile unsigned int reserved4 : 1;  // -
		volatile unsigned int acmd_err : 1;   // Auto command error (1 = error)
		volatile unsigned int reserved5 : 7;  // -
	} bits;
} interrupt_register;

// This register is used to mask interrupt flags in the interrupt_register (1 = yes, 0 = no)
typedef union {
	unsigned int raw;
	struct {
		volatile unsigned int cmd_done : 1;  // Set flag if command has finished
		volatile unsigned int data_done : 1; // Set flag if data transfer has finished
		volatile unsigned int block_gap : 1; // Set flag if data transfer stopped at block gap
		volatile unsigned int reserved : 1;  // -
		volatile unsigned int write_rdy : 1; // Set flag if data can be written to DATA register
		volatile unsigned int read_rdy : 1;  // Set flag if DATA register contains data to be read
		volatile unsigned int reserved2 : 2; // -
		volatile unsigned int card : 1;      // Set flag if card made interrupt request
		volatile unsigned int reserved3 : 3; // -
		volatile unsigned int retune : 1;    // Set  flag if clock retune request was made
		volatile unsigned int bootack : 1;   // Set flag if boot acknowledge has been received
		volatile unsigned int endboot : 1;   // Set flag if boot operation has terminated
		volatile unsigned int reserved4 : 1; // -
		volatile unsigned int cto_err : 1;   // Set flag if timeout on command line
		volatile unsigned int ccrc_err : 1;  // Set flag if command crc error
		volatile unsigned int cend_err : 1;  // Set flag if end bit on command line not 1
		volatile unsigned int cbad_err : 1;  // Set flag if incorrect command index in response
		volatile unsigned int dto_err : 1;   // Set flag if timeout on data line
		volatile unsigned int dcrc_err : 1;  // Set flag if data CRC error
		volatile unsigned int dend_err: 1;   // Set flag if end bit on data line not 1
		volatile unsigned int reserved5 : 1; // -
		volatile unsigned int acmd_err : 1;  // Set flag if auto command error
		volatile unsigned int reserved6 : 7; // -
	} bits;
} irpt_mask_register;

// irpt_en_register - Used to enable interrupts in interrupt_register
typedef union {
	volatile unsigned int raw;
	struct {
		volatile unsigned int cmd_done : 1;  // Create interrupt if command has finished
		volatile unsigned int data_done : 1; // Create interrupt if data transfer has finished
		volatile unsigned int block_gap : 1; // Create interrupt if data transfer stopped at block gap
		volatile unsigned int reserved : 1;  // -
		volatile unsigned int write_rdy : 1; // Create interrupt if data can be written to DATA register
		volatile unsigned int read_rdy : 1;  // Create interrupt if DATA register contains data to be read
		volatile unsigned int reserved2 : 2; // -
		volatile unsigned int card : 1;      // Create interrupt if card made interrupt request
		volatile unsigned int reserved3 : 3; // -
		volatile unsigned int retune : 1;    // Create interrupt if clock retune request was made
		volatile unsigned int bootack : 1;   // Create interrupt if boot acknowledge has been received
		volatile unsigned int endboot : 1;   // Create interrupt if boot operation has terminated
		volatile unsigned int reserved4 : 1; // -
		volatile unsigned int cto_err : 1;   // Create interrupt if timeout on command line
		volatile unsigned int ccrc_err : 1;  // Create interrupt if command crc error
		volatile unsigned int cend_err : 1;  // Create interrupt if end bit on command line not 1
		volatile unsigned int cbad_err : 1;  // Create interrupt if incorrect command index in response
		volatile unsigned int dto_err : 1;   // Create interrupt if timeout on data line
		volatile unsigned int dcrc_err : 1;  // Create interrupt if data CRC error
		volatile unsigned int dend_err: 1;   // Create interrupt if end bit on data line not 1
		volatile unsigned int reserved5 : 1; // -
		volatile unsigned int acmd_err : 1;  // Create interrupt if auto command error
		volatile unsigned int reserved6 : 7; // -
	} bits;
} irpt_en_register;

typedef enum {
	SDR12 = 0,
	SDR25 = 1,
	SDR50 = 2,
	SDR104 = 3,
	DDR50 = 4
} uhsmode;

// control2_register (1 = error, 0 = no error) - Used to enable different interrupts in interrupts_register
typedef union {
	unsigned int raw;
	struct {
		volatile unsigned int acnox_err : 1;   // Auto command not executed due to an error
		volatile unsigned int acto_err : 1;    // Timeout occurred during auto command
		volatile unsigned int accrc_err : 1;   // Command CRC error occurred during auto command execution
		volatile unsigned int acend_err : 1;   // End bit is not 1 during auto command execution
		volatile unsigned int acbad_err : 1;   // Command index error occurred during auto command execution
		volatile unsigned int reserved : 2;    // -
		volatile unsigned int notc12_err : 1;  // Error occurred during auto command CMD12 execution
		volatile unsigned int reserved2 : 8;   // -
		uhsmode uhsmode : 3;                   // Select the speed mode of the card
		volatile unsigned int reserved3 : 3;   // -
		volatile unsigned int tuneon : 1;      // Start tuning the SD clock (0 = not tuned/complete, 1 = tuning)
		volatile unsigned int tuned : 1;       // Tuned clock is used for sampling data (1 = yes)
		volatile unsigned int reserved4 : 8;    // -
	} bits;
} control2_register;

// force_irpt_register - Used to force fake interrupts for debugging
typedef union
{
	unsigned int raw;
	struct {
		volatile unsigned int cmd_done : 1;  // Command has finished
		volatile unsigned int data_done : 1; // Data transfer has finished
		volatile unsigned int block_gap : 1; // Data transfer stopped at block gap
		volatile unsigned int reserved : 1;  // -
		volatile unsigned int write_rdy : 1; // Data can be written to DATA register
		volatile unsigned int read_rdy : 1;  // Data register contains data to be read
		volatile unsigned int reserved2 : 2; // -
		volatile unsigned int card : 1;      // Card made interrupt request
		volatile unsigned int reserved3 : 3; // -
		volatile unsigned int retune : 1;    // Clock retune request was made
		volatile unsigned int bootack : 1;   // Boot acknowledge has been received
		volatile unsigned int endboot : 1;   // Boot operation has terminated
		volatile unsigned int reserved4 : 1; // -
		volatile unsigned int cto_err : 1;   // Timeout on command line
		volatile unsigned int ccrc_err : 1;  // Command crc error
		volatile unsigned int cend_err : 1;  // End bit on command line not 1
		volatile unsigned int cbad_err : 1;  // Incorrect command index in response
		volatile unsigned int dto_err : 1;   // Timeout on data line
		volatile unsigned int dcrc_err : 1;  // Data CRC error
		volatile unsigned int dend_err: 1;   // End bit on data line not 1
		volatile unsigned int reserved5 : 1; // -
		volatile unsigned int acmd_err : 1;  // Auto command error
		volatile unsigned int reserved6 : 7; // -
	} bits;
} force_irpt_register;

// Emmc
typedef struct { // Placed at EMMC_BASE
	volatile unsigned int Arg2;            // ACMD23 Argument
	blksizecnt_register BlockCountSize;    // Block size and count
	volatile unsigned int Arg1;            // Argument
	cmdtm_register Cmdtm;                  // Command and transfer mode
	volatile unsigned int Resp0;           // Response bits 31 : 0 - Cast this address to one of the SdResponseX
	volatile unsigned int Resp1;           // Response bits 63 : 32
	volatile unsigned int Resp3;           // Response bits 95 : 64
	volatile unsigned int Resp4;           // Response bits 127 : 96
	volatile unsigned int Data;            // Data
	emmc_status_register Status;           // Status
	control0_register Control0;            // Host configuration 0
	control1_register Control1;            // Host configuration 1
	interrupt_register Interrupt;          //    Interrupt Flags
	irpt_mask_register IrptMask;           //    Interrupt flag enable
	irpt_en_register IrptEn;               //    Interrupt Generation Enable
	control2_register Control2;            // Host configuration 2
	force_irpt_register ForceIrpt;         // Force interupt event (faking interrupts for debugging)
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
	unsigned char end:1;
} SdResponse1;

// SdResponse2 (136-Bit)
typedef struct { // 136-bit
	unsigned char start:1;
	unsigned char transmission:1;
	unsigned char reserved:6;
	unsigned int Cid[4];
	unsigned char end:1;
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
	unsigned int end:1;
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
	unsigned char end:1;
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
