#include "hardware/device/blockDevice.h"
#include "hardware/emmc.h"
#include "memory.h"
#include "hardware/device/sdBlockDevice.h"
#include "util/utilities.h"
#include "types/string.h"

Emmc* _emmc;

unsigned int Sd_Register(BlockDevice* device)
{
    my_memcpy(device->name, "SDBLKDEV\0", 9);
    device->name_length = 10;
    device->init = Sd_Initialize;
    device->cleanup = Sd_Cleanup;
    device->operation = Sd_DeviceOperation;
    device->buffer = (unsigned char*)palloc(BLOCK_SIZE);

    return 0;
}

int Sd_Initialize(void)
{
    int result = Emmc_Initialise();

    if (result != 0) {
        printf("sd - Failed to initialize emmc\n");
    }

    // Just do a dummy read on the first sector
    // To check that we can read

    char* buffer = (char*)palloc(BLOCK_SIZE);
    if ((int)Emmc_ReadBlock(buffer, BLOCK_SIZE, 0) != BLOCK_SIZE) 
        printf("dummy read in SD Initialize failed :(");

    phree(buffer);

    return result;
}

unsigned int Sd_DeviceOperation(BlockDevOp opCode, void* arg, void *arg2)
{
    switch (opCode)
    {
    case OpRead:
        printf("sd - reading a block\n");
        return Emmc_ReadBlock((char*)arg2, BLOCK_SIZE, *(unsigned int*)arg);
    case OpWrite:
        printf("Sd write is not yet implemented.\n");
        return 2;
    case OpGetStatus:
        return 0;
    default:
        return 4;
        break;
    }
}

void Sd_Cleanup(void)
{
    // Not implemented for Sd - What would we do?
}