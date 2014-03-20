#include "fs/filesystem.h"
#include "types/types.h"
#include "memory.h"
#include "util/utilities.h"
#include "types/string.h"
#include "fs/fat32.h"

static FileSystem* gFileSystem;

unsigned int Fs_Initialize(BlockDevice* device)
{
    unsigned int result = 0;
    LogicalDevice* logical = 0;

    // First time around - Allocate global structure
    if (!gFileSystem)
        gFileSystem = (FileSystem*)palloc(sizeof(FileSystem));

    // Store device as dev0 or dev1?
    unsigned int deviceId;
    if (!gFileSystem->devices[0].initialized)
    {
        logical = &gFileSystem->devices[0];
        deviceId = 0;
    }
    else if (!gFileSystem->devices[1].initialized)
    {
        logical = &gFileSystem->devices[1];
        deviceId = 1;
    }
    else
        return 2; // Error: Can't initialize more than two devices

    // Parse MBR and initialize partitions (if any)
    unsigned int sectorToRead = 0;
    ReturnOnFailure(result = device->operation(OpRead, &sectorToRead, device->buffer), "Failed to read MBR block\n");

    // Copy the mbr over to our struct as the buffer gets reused
    my_memcpy(&logical->mbr.bootloader[0], device->buffer, 512);

    // verify MBR signature
    if (logical->mbr.signature != 0xAA55)
    {
        printf("Invalid MBR signature on %s.\n", device->name);
        return 3;
    }

    // Setup the partitions
    unsigned int i;
    for (i = 0; i < 4; i++)
    {
        MbrPartitionEntry* part = &logical->mbr.partitions[i];

        if (logical->mbr.partitions[i].type == UnknownFsType)
            break; // No more partitions

        // Let the filesystem know where to start reading
        logical->partitions[i].firstSector = byte_to_int(part->lbaBegin);

        // TODO: Lookup registrered file systems keyed on type and use that instead
        //       Of hardcoding fat32 access here
        switch (part->type)
        {
        case FAT32WithCHS:
        case Fat32WithLba2:
        case Fat32WithLba3:
        case FAT32XWIthLBA:
            //printf("Fat32 partition detected on %s, type: %d.\n", device->name, part->type);
            logical->partitions[i].ownerDeviceId = deviceId;
            logical->partitions[i].partitionId = deviceId + i;
            logical->partitions[i].type = FAT32WithCHS;
            logical->partitions[i].device = device;
            Fat32_Initialize(&logical->partitions[i]);
            break;
        default:
            printf("Unknown partition type detected on %s. Type: %d.\n", device->name, part->type);
            break;
        }
    }

    logical->initialized = 1; // Looks good!

fExit:
    return result;
}

unsigned int Fs_getPartition(char* filename, Partition** part)
{
    unsigned int deviceId = *filename - 48;

    unsigned int physicalDeviceId = deviceId / 4;

    if (physicalDeviceId > 4 || gFileSystem->devices[physicalDeviceId].initialized != 1)
        return -1; // Invalid device Id

    *part = &(gFileSystem->devices[physicalDeviceId].partitions[physicalDeviceId % 4]);

    return 0;
}

int Fs_Open(char* filename, FileSystemOpenMode mode)
{
    int result = 0;
    Partition* part = 0;

    ReturnOnFailureF(result = Fs_getPartition(filename, &part), "Failed to get partition for '%s'.\n", filename);

    switch (part->type)
    {
    case FAT32WithCHS:
        return Fat32_Open(part, filename, mode);
    default:
        printf("Invalid filesystem type of partition.\n");
        break;
    }

fExit:
    return result;
}

int Fs_Close(int handle)
{
    int result = 0;

    unsigned int deviceId = (handle >> 16) & 0xFF;
    unsigned int partitionId = (handle >> 8) & 0xFF;

    ReturnOnFailure(result = deviceId > 4, "Invalid device id for handle.\n");
    ReturnOnFailure(result = partitionId > 4, "Invalid device id for handle.\n");

    Partition* part = &(gFileSystem->devices[deviceId].partitions[partitionId]);

    return part->close(part, handle);
/*
    switch (part->type)
    {
    case FAT32WithCHS:
        Fat32_Close(part, handle);
        break;
    default:
        printf("fs_close: Invalid partition type.\n");
        break;
    }*/

fExit:
    return result;
}

int Fs_Seek(int handle, long int offset, FsSeekOrigin origin)
{
    int result = 0;

    unsigned int deviceId = (handle >> 16) & 0xFF;
    unsigned int partitionId = (handle >> 8) & 0xFF;

    ReturnOnFailure(result = deviceId > 4, "Invalid device id for handle.\n");
    ReturnOnFailure(result = partitionId > 4, "Invalid device id for handle.\n");

    Partition* part = &(gFileSystem->devices[deviceId].partitions[partitionId]);    
    switch (part->type)
    {
    case FAT32WithCHS:
        Fat32_Seek(part, handle, offset, origin);
        break;
    default:
        printf("fs_close: Invalid partition type.\n");
        break;
    }

fExit:
    return result;
}

int Fs_Tell(int handle)
{
    unsigned int deviceId = (handle >> 16) & 0xFF;
    unsigned int partitionId = (handle >> 8) & 0xFF;

    Partition* part = &(gFileSystem->devices[deviceId].partitions[partitionId]);

    switch (part->type)
    {
    case FAT32WithCHS:
        return Fat32_Tell(part, handle);
        break;
    default:
        printf("fs_tell: Invalid partition type.\n");
        return -1;
    }
}

int Fs_Read(void* buf, unsigned long int bytesToRead, int handle)
{
    unsigned int deviceId = (handle >> 16) & 0xFF;
    unsigned int partitionId = (handle >> 8) & 0xFF;

    Partition* part = &(gFileSystem->devices[deviceId].partitions[partitionId]);
    
    return part->read(part, buf, bytesToRead, handle);

    /*switch (part->type)
    {
    case FAT32WithCHS:
        return Fat32_Read(part, buf, bytesToRead, handle);
        break;
    default:
        printf("fs_read: Invalid partition type.\n");
        break;
    }
    return -1;*/
}
