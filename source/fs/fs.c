#include "fs/fs.h"
#include "memory.h"
#include "util/utilities.h"
#include "types/string.h"

static file_system* gFs;

int fs_initialize(void)
{
    gFs = (file_system*)pcalloc(sizeof(file_system), 1);
}

int fs_register_driver(fs_driver* driver)
{
    // Todo, support removal?
    gFs->fs_drivers[gFs->num_drivers++] = driver;
}

int fs_add_device(BlockDevice* dev)
{
    int result = 0;

    // TODO: Add "initialized" flag?
    ReturnOnFailureF((result = dev->init()), 
        "fs - Failed to initialize device '%s'\n", dev->name);

    // Get first available storage device
    int dev_slot = -1;
    if (gFs->numDevices == 0)
    {
        // Great, just use the first slot!
        dev_slot = 0;
    }
    else
    {
        unsigned int i; // Linear search for first available slot
        for (i = 0; i < gFs->numDevices; i++) 
        {
            if (!gFs->devices[i] == 0)
            {
                dev_slot = i;
                break;
            }
        }
    }

    ReturnOnFailure(result = dev_slot == -1, "No storage device slots available\n");

    gFs->devices[dev_slot] = (storage_device*)pcalloc(sizeof(storage_device), 1);
    storage_device* storage = gFs->devices[dev_slot];
    
    // Read MBR
    unsigned int sector_to_read = 0;
    ReturnOnFailure(result = dev->operation(OpRead, &sector_to_read, dev->buffer), "Failed to read MBR partition\n");

    // Validate MBR signature
    mbr_t mbr;
    my_memcpy(&mbr, dev->buffer, 512); // Copy into an easy to read struct
    ReturnOnFailure(mbr.signature != 0xAA55, "Invalid MBR signature");

    // Parse partitions
    unsigned int i;
    for (i = 0; i < 4; i++) // NOTE: No support for extended partitions!
    {
        part_info* p = &mbr.partitions[i];

        if (p->type == unknown)
            break; // No more partitions

        partition* part = (partition*)pcalloc(sizeof(partition), 1);
        storage->partitions[storage->num_partitions++] = part;

        // Storage the partition info
        part->info = (part_info*)pcalloc(sizeof(part_info), 1);
        my_memcpy(part->info, p, sizeof(part_info));

        // Find driver that supports reading from this type
        fs_driver* driver = 0;
        unsigned int j;
        for (j = 0; j < gFs->num_drivers; j++)
        {
            if (gFs->fs_drivers[j]->type == p->type)
                driver = gFs->fs_drivers[j];
        }

        ReturnOnFailureF((result = driver != 0), "Not fs driver registered for partition of type %d\n", part->type);

        // Great! We've got a driver

        storage->partitions[storage->num_partitions - 1]->driver = driver;

        // Initialize the driver for this device
        
        driver->init(dev); // TODO: It might already be initialized

        // Uuuuh,,, how do we know if this combination has been intialzied?
        // Also the same BlockDevice can be used to rea dmultiple filesystems
        // by multiple drivers.... Figure this out.
    }

fExit:
    return result;
}
