
#define MAILBOX_FULL 0x80000000
#define MAILBOX_EMPTY 0x40000000

#define REQUEST_FLAG     0x00000000
#define RESPONSE_SUCCESS 0x80000000
#define RESPONSE_FAILURE 0x80000001

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

unsigned int Mailbox_SD_GetBaseFrequency(void);
