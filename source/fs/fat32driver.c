#include "fs/fat32driver.h"
#include "types/string.h"

static int fat32_driver_open()
{
    return -1;
}

static int fat32_driver_close()
{
    return -1;
}

static int fat32_driver_read()
{
    return -1;
}

static int fat32_driver_peek()
{
    return -1;
}

static int fat32_driver_tell()
{
    return -1;
}

static int fat32_driver_seek()
{
    return -1;
}

static void read_boot_sector(fat_boot_sector* boot_sector, char* buffer)
{
    // I'm currently doing this in a really nasty way at the moment because
    // I want the partition name for to be null terminated, and also because I'm
    // Not storing all information in the boot sector, like the bootloader code
    // TODO: Make the fat_boot_sector struct perfectly match what's on disk
    // So that this function can be replaced with a single call to memcpy()
    my_memcpy(&boot_sector->partition_type_name[0], &buffer[0x3], 8);
    boot_sector->partition_type_name[8] = '\0'; // Make oem name print friendly
    my_memcpy(&boot_sector->bytes_per_sector, &buffer[0x0B], 2);
    my_memcpy(&boot_sector->sectors_per_cluster, &buffer[0x0D], 1);
    my_memcpy(&boot_sector->num_reserved_sectors, &buffer[0x0E], 2);
    my_memcpy(&boot_sector->num_fats, &buffer[0x10], 1);
    my_memcpy(&boot_sector->root_entries, &buffer[0x11], 2);
    my_memcpy(&boot_sector->small_sectors, &buffer[0x13], 2);
    my_memcpy(&boot_sector->media_type, &buffer[0x15], 1);
    my_memcpy(&boot_sector->sectors_per_fat, &buffer[0x24], 2);
    my_memcpy(&boot_sector->sectors_per_head, &buffer[0x18], 2);
    my_memcpy(&boot_sector->number_of_heads, &buffer[0x1A], 2); // heads per cylinder
    my_memcpy(&boot_sector->hidden_sectors, &buffer[0x1C], 4);
    my_memcpy(&boot_sector->large_sectors, &buffer[0x20], 4);
    my_memcpy(&boot_sector->large_sectors_per_fat, &buffer[0x24], 4);
    my_memcpy(&boot_sector->flags, &buffer[0x28], 2);
    my_memcpy(&boot_sector->version, &buffer[0x2A], 2);
    my_memcpy(&boot_sector->root_dir_start, &buffer[0x2C], 4);
    my_memcpy(&boot_sector->info_sector, &buffer[0x30], 2);
    my_memcpy(&boot_sector->backup_sector, &buffer[0x32], 2);
}

int fat32_driver_factory(BlockDevice* device, part_info* pInfo, fs_driver_info** driver_info)
{
    // Check if we can handle the type
    if (pInfo->type != FAT32WithCHS && pInfo->type != Fat32WithLba3 && pInfo->type != FAT32XWithLba)
        return E_UNSUPPORTED;

    fat32_driver_info* info = (fat32_driver_info*)pcalloc(sizeof(fat32_driver_info), 1);
    info->basic.device = device;
    info->basic.info = pInfo;
    info->basic.operation = fat32_driver_operation;

    // Read the boot sector from the device
    unsigned int firstSectorToRead = byte_to_int(pInfo->lba_begin);
    if (device->operation(OpRead, &firstSectorToRead, device->buffer) != S_OK)
    {
        printf("Failed to read fat32 boot sector at sector %d\n", firstSectorToRead);
        phree(info);

        return E_GENERIC_ERROR; // TODO: Return a more descriptive error?
    }

    // Parse it into our structure
    read_boot_sector(&info->boot_sector, device->buffer);
    
    // Assign the return value
    *driver_info = info;
   
    return 0;
}

int fat32_driver_operation(fs_driver_info* info, fs_op operation, void* arg1, void* arg2)
{
    switch (operation)
    {
    case fs_op_read:
        return fat32_driver_read();
    case fs_op_open:
        return fat32_driver_open();
    case fs_op_close:
        return fat32_driver_close();
    case fs_op_seek:
        return fat32_driver_seek();
    case fs_op_tell:
        return fat32_driver_tell();
    case fs_op_peek:
        return fat32_driver_peek();
    default:
        // Not supported - Probably write?
        return -1;
    }
}
