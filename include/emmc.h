#define EMMC_BASE 0x20300000

#define SD_CMD_RSPNS_TYPE_NONE	0			// For no response
#define SD_CMD_RSPNS_TYPE_136	(1 << 16)		// For response R2 (with CRC), R3,4 (no CRC)
#define SD_CMD_RSPNS_TYPE_48	(2 << 16)		// For responses R1, R5, R6, R7 (with CRC)
#define SD_CMD_RSPNS_TYPE_48B	(3 << 16)		// For responses R1b, R5b (with CRC)
#define SD_CMD_RSPNS_TYPE_MASK  (3 << 16)

#define SD_CMD_CRCCHK_EN	(1 << 19)
#define SD_RESP_NONE        SD_CMD_RSPNS_TYPE_NONE
#define SD_RESP_R1          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R1b         (SD_CMD_RSPNS_TYPE_48B | SD_CMD_CRCCHK_EN)
#define SD_RESP_R2          (SD_CMD_RSPNS_TYPE_136 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R3          SD_CMD_RSPNS_TYPE_48
#define SD_RESP_R4          SD_CMD_RSPNS_TYPE_136
#define SD_RESP_R5          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R5b         (SD_CMD_RSPNS_TYPE_48B | SD_CMD_CRCCHK_EN)
#define SD_RESP_R6          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R7          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)

#define IS_APP_CMD              0x80000000

// Defines for setting commands
#define SD_CMD_DAT_DIR_HC	0
#define SD_CMD_BLKCNT_EN	(1 << 1)
#define SD_CMD_DAT_DIR_CH	(1 << 4)
#define SD_CMD_MULTI_BLOCK	(1 << 5)
#define SD_CMD_ISDATA       (1 << 21)
#define SD_CMD_TYPE_ABORT   (3 << 22)
#define SD_COMMAND_INDEX(x) (x << 24)
#define SD_CMD_RESERVED(x) 0xFFFFFFFF
#define SD_DATA_READ (SD_CMD_ISDATA | SD_CMD_DAT_DIR_CH)
#define SD_DATA_WRITE (SD_CMD_ISDATA | SD_CMD_DAT_DIR_HC)

#define SD_CARD_INTERRUPT       (1 << 8)

typedef enum {
	SdVersionUnknown = 0,
	SdVersion1 = 1,
	SdVersion1_1 = 2,
	SdVersion2 = 3,
	SdVersion3 = 4,
	SdVersion4 = 5

} SdVersion;

typedef struct
{
    unsigned int scr[2];
    unsigned int sd_bus_widths;
    int         sd_version;
}sd_scr;

typedef struct {
	sd_scr scr;
	unsigned int last_resp0;
	unsigned int last_resp1;
	unsigned int last_resp2;
	unsigned int last_resp3;

	unsigned int block_size;
	unsigned int blocks_to_transfer;

	unsigned int cid[4];
	unsigned int* receive_buffer;
	unsigned int rca;
	unsigned int supports_sdhc;
	unsigned int ocr;
} sd;

// SdClockSpeed - SD Clock Frequencies (in Hz)
typedef enum {
	SdClockId       = 400000,
	SdClockNormal   = 25000000,
	SdClockHigh     = 50000000,
	SdClock100      = 100000000,
	SdClock208      = 208000000
} SdClockSpeed;

// emmc_status_register
typedef volatile union{
	unsigned int raw;
	struct{
		unsigned int CmdInhibit : 1;     // Command line still in use (1 = yes)
		unsigned int DatInhibit : 1;     // Data line still in use (1 = yes)
		unsigned int DatActive : 1;      // At least one data line is active (1 = yes)
		unsigned int reserved : 5;       // -
		unsigned int WriteTransfer : 1;  // New data can be written to the EMMC
		unsigned int ReadTransfer : 1;   // New data can be read from EMMC
		unsigned int reserved2 : 10;     // -
		unsigned int DatLevel0 : 4;      // Values of DAT3 to DAT0
		unsigned int CmdLevel : 1;       // Value of command line CMD
		unsigned int DatLevel1 : 4;      // Value of data lines DAT7 to DAT4
		unsigned int reserved3 : 3;      // -
	} bits;
} emmc_status_register;

// blksizecnt_register
typedef volatile union {
	unsigned int raw;
	struct {
		unsigned int BlkSize : 10;
		unsigned int reserved : 6;
		unsigned int BlkCnt : 16;
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
typedef volatile union {
	unsigned int raw;
	struct {
		unsigned int reserved : 1;      // -
		unsigned int TmBlkCntEn : 1;    // Enable block counter for multiple block transfers (1 = eanble)
		tm_auto_cmd_en TmAutoCmdEn : 2; // Select command to send after completion of data transfer
		unsigned int TmDatDir : 1;      // Direction of data transfer (0 = host -> Card, 1 = card -> host)
		unsigned int TmMultiBlock : 1;  // Type of transfer (0 = single, 1 = multi)
		unsigned int reserved2 : 10;    // -
		cmd_rspns_type CmdRspnsType : 2;// Type of the response (see cmd_rspns_type)
		unsigned int CmdCrcChkEn : 1;   // Check the responses CRC (1 = enable)
		unsigned int CmdIxchkEn : 1;    // Check index has same index as command 
		unsigned int CmdIsData : 1;     // 0 = no data transfer command
		cmd_type CmdType : 2;           // The type of the command to be issued
		unsigned int CmdIndex : 6;      // Index of command to be issued
		unsigned int reserved3 : 2;     // -
	} bits;
} cmdtm_register;

// control0_register
typedef volatile union {
	unsigned int raw;
	struct {
		unsigned int hctl_dwidth : 1; // Use 4 data lines (1 = enabled)
		unsigned int hctl_hs_en : 1;  // Select high speed mode (i.e. DAT and CMD lines change on the rising CLK edge) (1 = enabled)
		unsigned int reserved : 2;    // -
		unsigned int hctl_8bit : 1;   // Use 8 data lines (1 = enabled)
		unsigned int reserved2 : 11;  // -
		unsigned int gap_stop : 1;    // Stop the current transaction at the next block gap (0 = ignore, 1 = stop)
		unsigned int gap_restart : 1; // Restart a transaction which was stopped using the gap_stop bit (0 = ignore, 1 = restart)
		unsigned int readwait_en : 1; // Use DAT2 read-wait protocol for SDIO cards supporting this (1 = enabled)
		unsigned int gap_ien : 1;     // Enable SDIO interrupt at block gap (only if the hctl_dwidth bit is set) (1 = enabled)
		unsigned int spi_mode : 1;    // SPI mode enable (0 = normal, 1 = SPI)
		unsigned int boot_en : 1;     // boot mode access (0 = stop boot mode access, 1 = start boot mode access)
		unsigned int alt_boot_en : 1; // Enable alternative boot mode access (1 = enabled)
		unsigned int reserved3 : 9;   // -
	} bits;
} control0_register;

// control1_register
typedef volatile union {
	unsigned int raw;
	struct {
		unsigned int clk_intlen : 1;  // Clock enable for internal emmc clocks for power saving (1 = enabled)
		unsigned int clk_stable : 1;  // SD clock stable (0 = no)
		unsigned int clk_en : 1;      // SD clock enable (1 = enabled)
		unsigned int reserved : 2;    // -
		unsigned int clk_gensel : 1;  // Mode of clock generation (0 = divided, 1 = programmable)
		unsigned int clk_freq_ms2 : 2;// SD clock base divider MSBs
		unsigned int clk_freq8 : 8;   // SD clock base divider LSBs
		unsigned int data_taunit : 4; // Data timeout unit exponent (1111 = disabled)
		unsigned int reserved2 : 4;   // -
		unsigned int srst_hc : 1;     // Reset the complete host circuit (1 = enabled)
		unsigned int srst_cmd : 1;    // Reset the command handler circuit (1 = enabled)
		unsigned int srst_data : 1;   // Reset the data handling circuit (1 = enabled)
		unsigned int reserved3 : 5;   // -
	} bits;
} control1_register;

// interrupt_register - Holds the interrupt flags, each flag can be disabled in IRPT_MASK
typedef volatile union {
	unsigned int raw;
	struct {
		unsigned int cmd_done : 1;   // Commnad has finished (1 = yes)
		unsigned int data_done : 1;  // Data transfer has finished (1 = yes)
		unsigned int block_gap : 1;  // Data transfer has stopped at block gap (1 = yes)
		unsigned int reserved : 1;   // -
		unsigned int write_rdy : 1;  // Data can be written to DATA register (1 = yes)
		unsigned int read_rdy : 1;   // Data register contains dta to be read (1 = yes)
		unsigned int reserved2 : 2;  // -
		unsigned int card : 1;       // Card made interrupt request (1 = yes)
		unsigned int reserved3 : 3;  // -
		unsigned int retune : 1;     // Clock retune request was made (1 = yes)
		unsigned int bootack : 1;    // Boot acknowledge has received (1 = yes)
		unsigned int endboot : 1;    // boot operation has terminated (1 = yes)
		unsigned int err : 1;        // An error has occurred (1 = error)
		unsigned int cto_err : 1;    // Timeout on command line (1 = error)
		unsigned int ccrc_err : 1;   // Command CRC error (1 = error)
		unsigned int cend_err : 1;   // End bit on command line not 1 (1 = error)
		unsigned int cbad_err : 1;   // Incorrect command index in response (1 = error)
		unsigned int dto_err : 1;    // Timeout on data line (1 = error)
		unsigned int dcrc_err : 1;   // Data CRC error (1 = error)
		unsigned int dend_error : 1; // End bit on data line not 1 (1 = error)
		unsigned int reserved4 : 1;  // -
		unsigned int acmd_err : 1;   // Auto command error (1 = error)
		unsigned int reserved5 : 7;  // -
	} bits;
} interrupt_register;

// This register is used to mask interrupt flags in the interrupt_register (1 = yes, 0 = no)
typedef volatile union {
	unsigned int raw;
	struct {
		unsigned int cmd_done : 1;  // Set flag if command has finished
		unsigned int data_done : 1; // Set flag if data transfer has finished
		unsigned int block_gap : 1; // Set flag if data transfer stopped at block gap
		unsigned int reserved : 1;  // -
		unsigned int write_rdy : 1; // Set flag if data can be written to DATA register
		unsigned int read_rdy : 1;  // Set flag if DATA register contains data to be read
		unsigned int reserved2 : 2; // -
		unsigned int card : 1;      // Set flag if card made interrupt request
		unsigned int reserved3 : 3; // -
		unsigned int retune : 1;    // Set  flag if clock retune request was made
		unsigned int bootack : 1;   // Set flag if boot acknowledge has been received
		unsigned int endboot : 1;   // Set flag if boot operation has terminated
		unsigned int reserved4 : 1; // -
		unsigned int cto_err : 1;   // Set flag if timeout on command line
		unsigned int ccrc_err : 1;  // Set flag if command crc error
		unsigned int cend_err : 1;  // Set flag if end bit on command line not 1
		unsigned int cbad_err : 1;  // Set flag if incorrect command index in response
		unsigned int dto_err : 1;   // Set flag if timeout on data line
		unsigned int dcrc_err : 1;  // Set flag if data CRC error
		unsigned int dend_err: 1;   // Set flag if end bit on data line not 1
		unsigned int reserved5 : 1; // -
		unsigned int acmd_err : 1;  // Set flag if auto command error
		unsigned int reserved6 : 7; // -
	} bits;
} irpt_mask_register;

// uhsmode
typedef enum {
	SDR12 = 0,
	SDR25 = 1,
	SDR50 = 2,
	SDR104 = 3,
	DDR50 = 4
} uhsmode;

// control2_register (1 = error, 0 = no error) - Used to enable different interrupts in interrupts_register
typedef volatile union {
	unsigned int raw;
	struct {
		unsigned int acnox_err : 1;   // Auto command not executed due to an error
		unsigned int acto_err : 1;    // Timeout occurred during auto command
		unsigned int accrc_err : 1;   // Command CRC error occurred during auto command execution
		unsigned int acend_err : 1;   // End bit is not 1 during auto command execution
		unsigned int acbad_err : 1;   // Command index error occurred during auto command execution
		unsigned int reserved : 2;    // -
		unsigned int notc12_err : 1;  // Error occurred during auto command CMD12 execution
		unsigned int reserved2 : 8;   // -
		uhsmode uhsmode : 3;          // Select the speed mode of the card
		unsigned int reserved3 : 3;   // -
		unsigned int tuneon : 1;      // Start tuning the SD clock (0 = not tuned/complete, 1 = tuning)
		unsigned int tuned : 1;       // Tuned clock is used for sampling data (1 = yes)
		unsigned int reserved4 : 8;   // -
	} bits;
} control2_register;

// exrdfifo_cfg_register
typedef volatile union {
	unsigned int raw;
	struct {
		unsigned int rd_thrsh : 3;  // The read threshold in 32 bits words
		unsigned int reserved : 29; // -
	} bits;
} exrdfifo_cfg_register;

// exrdfifo_en_register
typedef volatile union { 
	unsigned int raw;
	struct {
		unsigned int enable : 1;    // Enable the extension FIFO (1 = enabled)
		unsigned int reserved : 31; // -
	} bits;
} exrdfifo_en_register;

// tune_step
typedef enum {
	ps200 = 0,
	ps400 = 1,
	ps400_2 = 2,
	ps600 = 3,
	ps700 = 4,
	ps900 = 5,
	ps900_2 = 6,
	ps1100 = 7
} tune_step;

// tune_step_register
typedef volatile union {
	tune_step delay : 3;     // Sampling clock delay per step
	unsigned int reserved : 29; // -
} tune_step_register;

// tune_steps_std_register
typedef volatile union {
	unsigned int raw;
	struct {
		unsigned int steps : 6;     // Number of steps (0 to 40)
		unsigned int reserved : 26; // -
	} bits;
} tune_steps_std_register;

// tune_steps_ddr_register
typedef volatile union {
	unsigned int raw;
	struct {
		unsigned int steps : 6;     // Number of steps (0 to 40)
		unsigned int reserved : 26; // -
	} bits;
} tune_steps_ddr_register;

// spi_int_spt_register
typedef volatile union {
	unsigned int raw;
	struct {
		unsigned int select : 8;    // Interrupt independent of card select line (1 = yes)
		unsigned int reserved : 24; // -
	} bits;
} spi_int_spt_register;

// slotisr_ver_register
typedef volatile union {
	unsigned int raw;
	struct {
		unsigned int slot_status : 8;     // Logical OR of interrupt and wakeup signal for each slot
		unsigned int reserved : 8;        // -
		unsigned int sdversion : 8;       // Host controller specification version
		unsigned int vendor : 8;          // Vendor version number
	} bits;
} slotisr_ver_register;

// dbg_sel_register
typedef volatile union {
	unsigned int raw;
	struct {
		unsigned int select : 1;    // Submodules accessed by debug bus: 0 = receiver and fifo_ctrl, 1 = others
		unsigned int reserved : 31; // -
	} bits;
} dbg_sel_register;

// Emmc
typedef volatile struct { // Placed at EMMC_BASE
	unsigned int Arg2;                     // ACMD23 Argument
	blksizecnt_register BlockCountSize;    // Block size and count
	unsigned int Arg1;                     // Argument
	cmdtm_register Cmdtm;                  // Command and transfer mode
	unsigned int Resp0;                    // Response bits 31 : 0 - Cast this address to one of the SdResponseX
	unsigned int Resp1;                    // Response bits 63 : 32
	unsigned int Resp2;                    // Response bits 95 : 64
	unsigned int Resp3;                    // Response bits 127 : 96
	unsigned int Data;                     // Data
	emmc_status_register Status;           // Status
	control0_register Control0;            // Host configuration 0
	control1_register Control1;            // Host configuration 1
	interrupt_register Interrupt;          //   Interrupt Flags
	irpt_mask_register IrptMask;           //   Interrupt flag enable
	interrupt_register IrptEn;             //   Interrupt Generation Enable
	control2_register Control2; // 60      // Host configuration 2
	unsigned int Capabilities0;            // - Capabilities 0 (Not listed in BCM data sheet)
	unsigned int Capabilities1;            // - Capabilities 1 (Not listed in BCM data sheet)
	unsigned int reserved;                 // -
	unsigned int reserved1;                // -
	interrupt_register ForceIrpt;          // Force interrupt (faking interrupts for debugging)
	unsigned int reserved2;                // - 
	unsigned int reserved3;                // - 
	unsigned int reserved4;                // -
	unsigned int reserved5;                // -
	unsigned int reserved6;                // -
	unsigned int reserved7;                // -
	unsigned int reserved8;                // -
	unsigned int BootTimeout;              // Timeout in boot mode (number of card clock cycles after which a timeout during boot mode is flagged)
	dbg_sel_register DbgSel;               // Debug bus configuration
	unsigned int reserved9;                // -
	unsigned int reserved10;               // - 
	exrdfifo_cfg_register ExrdfifoCfg;     // 0x80 Extension FIFO configuration
	exrdfifo_en_register ExrdfifoEnable;   // Extension FIFO enable
	tune_step_register TuneStep;           // Delay per card clock turning step
	tune_steps_std_register TuneStepsStd;  // Card clock tuning steps for SDR
	tune_steps_ddr_register TuneStepsDdr;  // Card clock tuning steps for DDR
	unsigned int reserved11;               // -
	unsigned int reserved12;               // -
	unsigned int reserved13;               // -
	unsigned int reserved14;               // -
	unsigned int reserved15;               // -
	unsigned int reserved16;               // -
	unsigned int reserved17;               // -
	unsigned int reserved18;               // -
	unsigned int reserved19;               // -
	unsigned int reserved20;               // -
	unsigned int reserved21;               // -
	unsigned int reserved22;               // -
	unsigned int reserved23;               // -
	unsigned int reserved24;               // -
	unsigned int reserved25;               // -
	unsigned int reserved26;               // -
	unsigned int reserved27;               // -
	unsigned int reserved28;               // -
	unsigned int reserved29;               // -
	unsigned int reserved30;               // -
	unsigned int reserved31;               // -
	unsigned int reserved32;               // -
	unsigned int reserved33;               // -
	spi_int_spt_register SPIIntSpt;        // SPI Interrupt support
	unsigned int reserved34;               // -
	unsigned int reserved35;               // -
	slotisr_ver_register SlotisrVer;       // Slot interrupt status and version
} Emmc;

// EmmcCommand
typedef enum {	
	GoIdleState          = 0,	// Resets SD card
	SendOpCond           = 1,	// Sends host capacity support information and kick off card initialization
	AllSendCid           = 2,	// Broadcast: Request all cards to send cid
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
	unsigned int start:1;
	unsigned int transmission:1; // direction, 1 = host, 0 = card
	unsigned int commandIndex:6;
	unsigned int argument:32;
	unsigned int crc7:7;
	unsigned int end:1;
} SdCommand;

// SdCardStatus
typedef struct{
	unsigned int out_of_range : 1;
	unsigned int address_error : 1;
	unsigned int block_len_error: 1;
	unsigned int erase_seq_error : 1;
	unsigned int erase_param : 1;
	unsigned int wp_violation : 1;	
	unsigned int card_is_locked : 1;
	unsigned int lock_unlocked_failed : 1;
	unsigned int com_crc_error : 1;
	unsigned int illegal_command : 1;
	unsigned int card_ecc_failed : 1;
	unsigned int cc_error : 1;
	unsigned int error : 1;
	unsigned int reserved1 : 1;
	unsigned int reserved2 : 1;

	unsigned int csd_overwrite : 1;
	unsigned int wp_erase_skip : 1;
	unsigned int card_ecc_disabled : 1;	
	unsigned int erase_reset : 1;
	unsigned int current_state : 4;
	unsigned int ready_for_data : 1;
	unsigned int reserved3 : 1;
	unsigned int app_cmd : 1;
	unsigned int reserved4 : 1;

	unsigned int ake_seq_error : 1;
} SdCardStatus;

// SdResponse1 (48-Bit)
typedef struct { // 48-Bit
	unsigned int start:1;
	unsigned int transmission:1;
	unsigned int commandIndex:6;
	unsigned int cardStatus:32;
	unsigned int crc7:7;
	unsigned int end:1;
} SdResponse1;

// SdResponse2 (136-Bit)
typedef struct { // 136-bit
	unsigned int start:1;
	unsigned int transmission:1;
	unsigned int reserved:6;
	unsigned int Cid[4];
	unsigned int end:1;
} SdResponse2;

// SdResponse2 (48-Bit)
typedef struct { // 48-bit
	unsigned int start:1;
	unsigned int transmission:1;
	unsigned int reserved:6;
	unsigned int Ocr:32;
	unsigned int reserved2: 8;
	unsigned int end:1;
} SdResponse3;

// SdResponse6 (48-Bit)
typedef struct { // 48-Bit
	unsigned int start:1;
	unsigned int transmission:1;
	unsigned int commandIndex:6;
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
	unsigned int start:1;
	unsigned int tranmission:1;
	unsigned int commandIndex:6;
	unsigned int reserved:20;
	unsigned int voltageAccepted:4;
	unsigned int checkPatternEcho:8;
	unsigned int crc7:7;
	unsigned int end:1;
} SdResponse7;

unsigned int EmmcInitialise(void);
unsigned int EmmcGetClockSpeed(void);
unsigned int EmmcPowerOn(void);
unsigned int EmmcPowerOff(void);
unsigned int EmmcPowerCycle(void);
unsigned int EmmcSetClockRate(unsigned int clock, unsigned int targetRate);
unsigned int EmmcSendCommand(unsigned int cmd, unsigned int argument);
unsigned int EmmcRead(unsigned char* buf, unsigned int bufLen, unsigned int blockToReadFrom);
unsigned int EmmcWrite(unsigned char* buf, unsigned int bufLEn, unsigned int blockToWriteTo);
