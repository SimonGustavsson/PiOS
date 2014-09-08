
#define MAILBOX_FULL 0x80000000
#define MAILBOX_EMPTY 0x40000000

#define REQUEST_FLAG     0x00000000
#define RESPONSE_SUCCESS 0x80000000
#define RESPONSE_FAILURE 0x80000001

// Unique Voltage Ids to use with Get/Set voltage messages
typedef enum {
    MBV_RESERVED = 0x000000000,
    MBV_CORE     = 0x000000001,
    MBV_SDRAM_C  = 0x000000002,
    MBV_SDRAM_P  = 0x000000003,
    MBV_SDRAM_I  = 0x000000004
} mb_voltage_id;

typedef enum {
        MBC_ID_RESERVED = 0x000000000,
        MBC_ID_EMMC     = 0x000000001,
        MBC_ID_UART     = 0x000000002,
        MBC_ID_ARM      = 0x000000003,
        MBC_ID_CORE     = 0x000000004,
        MBC_ID_V3D      = 0x000000005,
        MBC_ID_H264     = 0x000000006,
        MBC_ID_ISP      = 0x000000007,
        MBC_ID_SDRAM    = 0x000000008,
        MBC_ID_PIXEL    = 0x000000009,
        MBC_ID_PWM      = 0x00000000A,
} mb_clock_id;

typedef enum
{
    MEM_FLAG_DISCARDABLE = 1 << 0, /* can be resized to 0 at any time. Use for cached data */
    MEM_FLAG_NORMAL = 0 << 2, /* normal allocating alias. Don't use from ARM */
    MEM_FLAG_DIRECT = 1 << 2, /* 0xC alias uncached */
    MEM_FLAG_COHERENT = 2 << 2, /* 0x8 alias. Non-allocating in L2 but coherent */
    MEM_FLAG_L1_NONALLOCATING = (MEM_FLAG_DIRECT | MEM_FLAG_COHERENT), /* Allocating in L2 */
    MEM_FLAG_ZERO = 1 << 4,  /* initialise buffer to all zeros */
    MEM_FLAG_NO_INIT = 1 << 5, /* don't initialise (default is initialise to all ones */
    MEM_FLAG_HINT_PERMALOCK = 1 << 6, /* Likely to be locked for long periods of time. */
} mb_mem_flags;

typedef enum {
    MBT_GET_BOARD_MODEL    = 0x00010001,
    MBT_GET_BOARD_REVISION = 0x00010002,
    MBT_GET_BOARD_MAC      = 0x00010003,
    MBT_GET_BOARD_SERIAL   = 0x00010004,
    MBT_GET_ARM_MEMORY     = 0x00010005,
    MBT_GET_VC_MEMORY      = 0x00010006,
    MBT_GET_CLOCKS         = 0x00010007,
    MBT_GET_CMDLINE        = 0x00050001,
    MBT_GET_DMA_CHANS      = 0x00060001,
    MBT_GET_POWER_STATE    = 0x00020001,
    MBT_GET_TIMING         = 0x00020002,
    MBT_SET_POWER_STATE    = 0x00028001,
    MBT_GET_CLOCK_STATE    = 0x00030001,
    MBT_SET_CLOCK_STATE    = 0x00038001,
    MBT_GET_CLOCKRATE      = 0x00030002,
    MBT_SET_CLOCKRATE      = 0x00038002,
    MBT_GET_CLOCKRATE_MAX  = 0x00030004,
    MBT_GET_CLOCKRATE_MIN  = 0x00030007,
    MBT_GET_TURBO          = 0x00030009,
    MBT_SET_TURBO          = 0x00038009,
    MBT_GET_VOLTAGE        = 0x00030003,
    MBT_SET_VOLTAGE        = 0x00038003,
    MBT_GET_VOLTAGE_MAX    = 0x00030005,
    MBT_GET_VOLTAGE_MIN    = 0x00030008,
    MBT_GET_TEMP           = 0x00030006,
    MBT_GET_TEMP_MAX       = 0x0003000A,
    MBT_ALLOC_MEMORY       = 0x0003000C,
    MBT_LOCK_MEMORY        = 0x0003000D,
    MBT_UNLOCK_MEMORY      = 0x0003000E,
    MBT_RELEASE_MEMORY     = 0x0003000F,
    MBT_EXECUTE_CODE       = 0x00030010,
    MBT_FB_ALLOC           = 0x00040001,
    MBT_FB_RELEASE         = 0x00048001,
    MBT_FB_BLANK           = 0x00040002,
    MBT_FB_GET_PHYSIZE     = 0x00040003,
    MBT_FB_TST_PHYSIZe     = 0x00044003,
    MBT_FB_SET_PHYSIZE     = 0x00048003,
    MBT_FB_GET_VIRTSIZE    = 0x00040004,
    MBT_FB_TST_VIRTSIZE    = 0x00044004,
    MBT_FB_SET_VIRTSIZE    = 0x00048004,
    MBT_FB_GET_DEPTH       = 0x00040005,
    MBT_FB_TST_DEPTH       = 0x00044005,
    MBT_FB_SET_DEPTH       = 0x00048005,
    MBT_FB_GET_PIXEL_ORDER = 0x00040006,
    MBT_FB_TST_PIXEL_ORDER = 0x00044006,
    MBT_FB_SET_PIXEL_ORDER = 0x00048006,
    MBT_FB_GET_ALPHA_MODE  = 0x00040007,
    MBT_FB_TST_ALPHA_MODE  = 0x00044007,
    MBT_FB_SET_ALPHA_MODE  = 0x00048007,
    MBT_FB_GET_PITCH       = 0x00040008,
    MBT_FB_GET_VIRT_OFFSET = 0x00040009,
    MBT_FB_TST_VIRT_OFFSET = 0x00044009,
    MBT_FB_SET_VIRT_OFFSET = 0x00048009,
    MBT_FB_GET_OVERSCAN    = 0x0004000A,
    MBT_FB_TST_OVERSCAN    = 0x0004400A,
    MBT_FB_SET_OVERSCAN    = 0x0004800a,
    MBT_FB_GET_PALETTE     = 0x0004000B,
    MBT_FB_TST_PALETTE     = 0x0004400B,
    MBT_FB_SET_PALETTE     = 0x0004800B,
} mailbox_tag;

// Device Ids for power management
typedef enum {
	HwId_Emmc = 0x00000000,
	HwId_Uart0 = 0x00000001,
	HwId_Uart1 = 0x00000002,
	HwId_UsbHcd = 0x00000003,
	HwId_I2c0 = 0x00000004,
	HwId_I2c1 = 0x00000005,
	HwId_I2c2 = 0x00000006,
	HwId_Spi = 0x00000007,
	HwId_Ccp2tx = 0x00000008
} HardwareId;

typedef enum {
	// Setting
	HwPowerState_OffDontWait = 0,
	HwPowerState_OffWait = 1,
	HwPowerState_OnDontWait = 2,
	HwPowerState_OnWait = 3,
	
	// Status result
	HwPowerState_OffExists = 0,
	HwPowerState_OffDoesntExist = 1,
	HwPowerState_OnExists = 2,
	HwPowerState_OnDoesntExist = 3,
} HardwarePowerState;

void Mailbox_Write(unsigned int channel, unsigned int data);
unsigned int Mailbox_Read(unsigned int channel);
unsigned int Mailbox_GetPowerState(unsigned int deviceId);
int Mailbox_SetDevicePowerState(unsigned int deviceId, unsigned int powerState);
int Mailbox_GetClockRate(unsigned int clockId);

unsigned int Mailbox_SdGetBaseFrequency(void);
