#include "fs/fs.h"
#include "memory.h"
#include "util/utilities.h"
#include "types/string.h"

static file_system* gFs;

int fs_initialize(void)
{
    gFs = (file_system*)pcalloc(sizeof(file_system), 1);

    return 0;
}

int fs_register_driver_factory(int(*factory)(BlockDevice* device, part_info* pInfo, fs_driver_info** driver_info))
{
    printf("Registering new fs driver factory, currently %d registered.\n", gFs->num_factories);

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
    printf("Getting next free logical device slot\n");

    unsigned int i;
    if (gFs->numDevices == 0)
    {
        printf("First registered device, using slot 0\n");
        return 0;
    }
    else
    {
        for(i = 0; i < gFs->numDevices; i++) 
        {
            if (gFs->devices[i] == 0)
            {
                printf("Found free slot %d\n", i);
                return i;
            }
        }
    }

    printf("No slots available for logical device\n");

    return -1; // no slots available
}

// This is an extremely stupid and ugly way of doing it, but I don't have sscanf yet
// Created a new name for a harddrive "hdd{i}" where {i} is the currently registered
// Device count
char* fs_get_next_partition_name(void)
{
    printf("Creating name for next partition\n");
    char* name = (char*)pcalloc(sizeof(char*), 10); // Max name length is 10, random hooo
    my_strcpy("hdd", name);

    // Append the device index to the name
    char* devIndexStr = (char*)pcalloc(sizeof(char), 5);
    itoa(gFs->numDevices - 1, devIndexStr);
    my_strcpy((const char*)devIndexStr, &name[3]);
    
    // Make sure it's 0-terminated
    name[3 + my_strlen(devIndexStr) + 1] = 0;

    printf("Name created: '%s'\n", name);

    return name;
}

int fs_add_device(BlockDevice* dev)
{
    printf("Adding block deice to file system\n");
    int result = 0;
    int dev_slot;
    unsigned int i;
    unsigned int sector_to_read;
    mbr_t mbr;
    part_info* pInfo;
    partition* part;
    fs_driver_info* driver = 0;
    unsigned int j;
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

    printf("MBR size: %d, part info size: %d\n", sizeof(mbr_t), sizeof(part_info));

    my_memcpy(&mbr, dev->buffer, 512); // Copy into an easy to read struct
    ReturnOnFailure(mbr.signature != 0xAA55, "Invalid MBR signature");

    printf("Valid MBR signature found, parsing partition table\n");
    // Parse partitions
    for (i = 0; i < 4; i++) // NOTE: No support for extended partitions!
    {
        pInfo = &mbr.partitions[i];

        if (pInfo->type == unknown)
        {
            printf("No more partitions\n");
            break; // No more partitions
        }

        printf("Finding driver for partition %d\n", i);

        // Find driver that supports reading from this type
        unsigned int j;
        for (j = 0; j < gFs->num_factories; j++)
        {
            if (gFs->factories[j](dev, pInfo, &driver) == 0)
            {
                printf("Factory successfully created a driver\n");
                break;
            }
        }

        printf("We should have called factory now if any can handle it, driver: 0x%h\n", driver);
        
        if (driver == 0)
        {
            printf("Not fs driver registered for partition of type %d\n", part->type);
            continue;
        }
        else
        {
            printf("Driver found , initializing partition structure\n");
        }

        // Driver has been initialized

        // Create part struct where we store stuff
        part = (partition*)pcalloc(sizeof(partition), 1);
        part->info = (part_info*)pcalloc(sizeof(part_info), 1);

        part->name = fs_get_next_partition_name();
         
        // Storage the partition info
        my_memcpy(part->info, pInfo, sizeof(part_info));

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
