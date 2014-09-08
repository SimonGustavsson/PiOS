#include "hardware/blockDevice.h"

unsigned int Sd_Register(BlockDevice* device);

int Sd_Initialize(void);
unsigned int Sd_DeviceOperation(BlockDevOp opCode, void* arg, void *arg2);
unsigned int Sd_GetSector(unsigned int sector, unsigned char* buf);
void Sd_Cleanup(void);