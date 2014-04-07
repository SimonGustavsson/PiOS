#include "fs/fs.h"
#include "memory.h"
#include "util/utilities.h"
#include "types/string.h"

static file_system* gFs;

/*
* Forward declaring 
*/
// NOTE: This function expects ABSOLUTE paths, /dev/hdd0/....
int fs_get_partition(char* filename, partition** part);

// NOTE: This function expects ABSOLUTE paths, /dev/hdd0/....
int fs_get_partition(char* filename, partition** part)
{
    // Skip past /dev/, we don't need it from this point onwards
    filename += 5;
    
    // Find the end of the device name (which is actually the partition)
    int deviceNameEndIndex = 0;
    while (*filename && *filename != '/')
    {
        filename++; 
        deviceNameEndIndex++;
    }

    // Rewind back so we can extract device name
    filename -= deviceNameEndIndex;

    // Strip out the device name
    int devNameLen = deviceNameEndIndex;
    char* devName = (char*)pcalloc(sizeof(char), deviceNameEndIndex + 2);
    my_memcpy(devName, filename, deviceNameEndIndex);
    devName[deviceNameEndIndex + 1] = 0;

    unsigned int i, j;
    for (i = 0; i < gFs->numDevices; i++)
    {
        for (j = 0; j < gFs->devices[i]->num_partitions; j++)
        {
            partition* curPart = gFs->devices[i]->partitions[j];
            if (curPart->name_len == devNameLen && my_strcmp_s(curPart->name, devNameLen, devName) == 0)
            {
                *part = curPart;

                // Return information about this partition that can be used to make up a file handle
                // Making it easy to retrieve this partition
                // Bits of handle:
                // uuuuuuuu dddddddd pppppppp oooooooo
                // u = Unused, d = device index, p = partition index, o = open file index
                // Note open file index is not set by this function, we just make room for it
                return ((i & 0xFF) << 16) | ((j & 0xFF) << 8);
            }
        }
    }

    printf("Could not find partition for filename\n");

    return INVALID_HANDLE;
}

int fs_initialize(void)
{
    gFs = (file_system*)pcalloc(sizeof(file_system), 1);

    return 0;
}

int fs_register_driver_factory(int(*factory)(BlockDevice* device, part_info* pInfo, fs_driver_info** driver_info))
{
    if (gFs->num_factories >= MAX_FS_DRIVER_FACTORIES)
    {
        printf("Can only register 4 driver factories\n");
        return -1;
    }

    gFs->factories[gFs->num_factories++] = factory;

    return 0;
}

int fs_get_next_free_device_slot()
{
    unsigned int i;
    if (gFs->numDevices == 0)
        return 0;
    else
    {
        for(i = 0; i < gFs->numDevices; i++) 
        {
            if (gFs->devices[i] == 0)
                return i;
        }
    }

    printf("No slots available for logical device\n");

    return -1; // no slots available
}

// This is an extremely stupid and ugly way of doing it, but I don't have sscanf yet
// Created a new name for a harddrive "hdd{i}" where {i} is the currently registered
// Device count
char* fs_get_next_partition_name(BlockDevice* dev)
{
    char* name = (char*)pcalloc(sizeof(char*), 10); // Max name length is 10, random hooo
    int typeLen = 0;

    if (dev->type == BlockDevRemovable)
    {
        my_strcpy("sd", name);
        typeLen = 2;
    }
    else
    {
        my_strcpy("hdd", name);
        typeLen = 3;
    }

    // Append the device index to the name
    char* devIndexStr = (char*)pcalloc(sizeof(char), 5);
    itoa(gFs->numDevices - 1, devIndexStr);
    my_strcpy((const char*)devIndexStr, &name[typeLen]);
    
    // Make sure it's 0-terminated
    name[typeLen + my_strlen(devIndexStr) + 1] = 0;

    return name;
}

int fs_add_device(BlockDevice* dev)
{
    int result = 0;
    int dev_slot;
    unsigned int i;
    unsigned int sector_to_read;
    mbr_t mbr;
    part_info* pInfo;
    partition* part;
    fs_driver_info* driver = 0;
    unsigned int num_valid_partitions = 0;

    // TODO: Add "initialised" flag?
    ReturnOnFailureF((result = dev->init()), "fs - Failed to initialise device '%s'\n", dev->name);

    dev_slot = fs_get_next_free_device_slot();
    ReturnOnFailure(result = (dev_slot == -1), "No storage device slots available\n");

    gFs->devices[dev_slot] = (storage_device*)pcalloc(sizeof(storage_device), 1);
    gFs->numDevices++; // A new one has been added!
    
    // Read MBR
    sector_to_read = 0;
    ReturnOnFailure(result = dev->operation(OpRead, &sector_to_read, dev->buffer), "Failed to read MBR partition\n");

    my_memcpy(&mbr, dev->buffer, 512); // Copy into an easy to read struct
    ReturnOnFailure(mbr.signature != 0xAA55, "Invalid MBR signature");

    // Parse partitions
    for (i = 0; i < 4; i++) // NOTE: No support for extended partitions!
    {
        pInfo = &mbr.partitions[i];

        if (pInfo->type == unknown)
            break; // No more partitions

        // Find driver that supports reading from this type
        unsigned int j;
        for (j = 0; j < gFs->num_factories; j++)
        {
            if (gFs->factories[j](dev, pInfo, &driver) == 0)
                break;
        }
        
        if (driver == 0)
        {
            printf("Not fs driver registered for partition of type %d\n", part->type);
            continue;
        }

        // Driver has been initialized

        // Create part struct where we store stuff
        part = (partition*)pcalloc(sizeof(partition), 1);
        
        part->name = fs_get_next_partition_name(dev);
        part->name_len = my_strlen(part->name);
        part->driver = driver;
        
        // Store partition in global struct
        gFs->devices[dev_slot]->partitions[gFs->devices[dev_slot]->num_partitions++] = part;

        num_valid_partitions++;

        printf("Partition '%s'(%d) all set up and ready!\n", part->name, i);
    }

    // If we got this far, we consider initialization a success, even if the device didn't
    // Contain any valid partitions
    result = 0;

fExit:
    return result;
}

int fs_get_direntry(partition* part, char* filename, direntry** fileEntry)
{
    return -1;
}

// NOTE: This assumes absolute paths like /dev/sd0/... for no
int fs_open(char* filename, file_mode mode)
{
    partition* part;

    int handle = fs_get_partition(filename, &part);

    if (handle == INVALID_HANDLE)
    {
        printf("No device with that name registered\n");
        return INVALID_HANDLE;
    }

    if (part->num_open_dirs >= 10)
    {
        printf("Can't have more than 10 files open at once, close one and try again.\n");
        return INVALID_HANDLE;
    }

    // Strip out device name and everything, the driver only needs to know the file name
    // Relative to its filesystem
    filename += 5;
    while (*filename && *filename++ != '/');
    
    direntry* fileEntry = 0;
    if (part->driver->operation(part->driver, fs_op_open, filename, &fileEntry, 0) != S_OK)
    {
        printf("Failed to get directory entry for '%s', does it really exist?\n", filename);
        return INVALID_HANDLE;
    }

    direntry_open* entry = (direntry_open*)pcalloc(sizeof(direntry_open), 1);
    entry->mode = mode;
    entry->offset = 0;
    entry->entry = fileEntry;

    // Find the first empty slot in our array to store it in (God I wish I had List<T>!)
    int freeIndex = -1;
    unsigned int i;
    for (i = 0; i < 10; i++)
    {
        if (part->open_dirs[i] == 0)
        {
            freeIndex = i;
            break;
        }
    }

    if (freeIndex == -1)
    {
        printf("Could not find an empty slot for direntry_open, what happened here?\n");
        phree(entry);
        return INVALID_HANDLE;
    }
    
    part->open_dirs[freeIndex] = entry;

    part->num_open_dirs++;

    // Add open filed index to the handle by sticking it in the low 8 bits and return it
    return freeIndex & 0xFF;
}

int fs_close(int handle)
{
    // Handles look like this:
    // uuuuuuuu dddddddd pppppppp oooooooo
    // u = Unused, d = device index, p = partition index, o = open file index
    if (handle == INVALID_HANDLE)
    {
        printf("Can't close invalid file handle\n");
        return -1;
    }

    int devIndex = (handle >> 16) & 0xFF;
    int partIndex = (handle >> 8) & 0xFF;
    int fileIndex = handle & 0xFF;

    if (devIndex == -1 || partIndex == -1 || fileIndex == -1 || 
        devIndex >= gFs->numDevices || partIndex >= gFs->devices[devIndex]->num_partitions)
    {
        printf("Can't close invalid file handle(d:%d,p:%d,f:%d)\n", devIndex, partIndex, fileIndex);
        return -1; // Invalid handle
    }

    partition* part = gFs->devices[devIndex]->partitions[partIndex];

    phree(part->open_dirs[fileIndex]);
    part->open_dirs[fileIndex] = 0;
    part->num_open_dirs--;

    return S_OK;
}

int fs_read(int handle, char* buffer, long long bytesToRead)
{
    int devIndex = (handle >> 16) & 0xFF;
    int partIndex = (handle >> 8) & 0xFF;
    int fileIndex = handle & 0xFF;

    if (devIndex == -1 || partIndex == -1 || fileIndex == -1 ||
        devIndex >= gFs->numDevices || partIndex >= gFs->devices[devIndex]->num_partitions)
    {
        printf("Can't close invalid file handle(d:%d,p:%d,f:%d)\n", devIndex, partIndex, fileIndex);
        return -1; // Invalid handle
    }

    partition* part = gFs->devices[devIndex]->partitions[partIndex];

    return part->driver->operation(part->driver, fs_op_read, part->open_dirs[fileIndex]->entry, buffer, bytesToRead);
}

long long fs_tell(int handle)
{

    int devIndex = (handle >> 16) & 0xFF;
    int partIndex = (handle >> 8) & 0xFF;
    int fileIndex = handle & 0xFF;

    if (devIndex == -1 || partIndex == -1 || fileIndex == -1 ||
        devIndex >= gFs->numDevices || partIndex >= gFs->devices[devIndex]->num_partitions)
    {
        printf("Could not find partition for handle %d\n", handle);
        return -1; // Invalid handle
    }

    partition* part = gFs->devices[devIndex]->partitions[partIndex];

    if (fileIndex == -1)
    {
        printf("Invalid handle\n");
        return -1;
    }

    direntry_open* de = part->open_dirs[fileIndex];

    return de->offset;
}

int fs_seek(int handle, long long offsetL, seek_origin origin)
{
    unsigned int offset = offsetL & 0xFFFFFFFF; // HAck, no long support?
    
    int devIndex = (handle >> 16) & 0xFF;
    int partIndex = (handle >> 8) & 0xFF;
    int fileIndex = handle & 0xFF;

    if (devIndex == -1 || partIndex == -1 || fileIndex == -1 ||
        devIndex >= gFs->numDevices || partIndex >= gFs->devices[devIndex]->num_partitions)
    {
        printf("Could not find partition for handle %d\n", handle);
        return -1; // Invalid handle
    }

    partition* part = gFs->devices[devIndex]->partitions[partIndex];
    direntry_open* de = part->open_dirs[fileIndex];

    direntry* entry = de->entry;
    unsigned int size = entry->size;

    switch (origin)
    {
    case seek_relative:
        de->offset = (de->offset + offset <= size) ? de->offset + offset : size;
        break;
    case seek_end:
        de->offset = size;
        break;
    case seek_begin:
        de->offset = (offset > size) ? size : offset;
        break;
    default:
        printf("Invalid seek origin passed to fs_seek\n");
        return -1;
    }

    return S_OK;
}