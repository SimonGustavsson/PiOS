#include "fs/fs.h"
#include "fs/fat32driver.h"
#include "types/string.h"

// Forward declare static functions
static direntry* fat32_listDirectory(fat32_driver_info* part, unsigned int firstSector, unsigned int* pFilesFound);
static int fat32_getFatEntry(fat32_driver_info* driver, unsigned int cluster);
int fat32_getDirEntry(fat32_driver_info* part, char* filename, direntry** entry);

static int fat32_getFatEntry(fat32_driver_info* driver, unsigned int cluster)
{
    int fatEntry = -1;

    unsigned int indexOfEntryInCluster = cluster % BLOCK_SIZE;
    unsigned int fatbegin = driver->first_sector + driver->boot_sector.num_reserved_sectors;
    unsigned int sectorToRead = fatbegin + (cluster / BLOCK_SIZE);

    int readResult;
    ReturnOnFailureF(readResult = driver->basic.device->operation(OpRead, &sectorToRead, driver->basic.device->buffer),
        "Failed to read fat table, %d\n", readResult);

    // Extract the entry from the block we just read
    fatEntry = byte_to_int(&driver->basic.device->buffer[indexOfEntryInCluster]);
    
fExit:
    return fatEntry;
}

// TODO: This must be updated to our new directory structure
static int fat32_translateTo83FatName(char* filename, char* dest)
{
    char* file = filename;

    // Split out extension from file name
    unsigned int nameLength = my_strlen(file);
    int extIndex;
    unsigned int hasExtension = 0;
    // Step backwards and find the first full stop
    for (extIndex = nameLength; extIndex >= 0; extIndex--)
    {
        if (*(file + extIndex) == '.')
        {
            hasExtension = 1;
            break; // Found it! :-)
        }
    }

    if (hasExtension)
    {
        my_memcpy(dest, file, extIndex);

        unsigned int i;
        int paddingSpacesNeeded = 11 - (nameLength - 1); // We removed the . remember?
        if (paddingSpacesNeeded > 0)
        {
            for (i = 0; i < (unsigned int)paddingSpacesNeeded; i++)
                *((dest + extIndex) + i) = ' ';
        }

        unsigned int extLength = nameLength - extIndex + 1;

        // and write extension
        for (i = 0; i < extLength; i++)
            *(dest + extIndex + paddingSpacesNeeded + i) = *(file + extIndex + 1 + i);

        // aaaand make it all upper case (Note: We need to start considering case insensitive stuff here...)
        for (i = 0; i < 11; i++)
        {
            if (*(dest + i) > 96 && *(dest + i) < 123)
                *(dest + i) = *(dest + i) - 32;
        }
    }
    else
    {
        // No extension

        // Copy the entire name over
        my_memcpy(dest, file, nameLength);

        // Make upper case
        unsigned int i;
        for (i = 0; i < 11; i++)
        {
            if (*(dest + i) > 96 && *(dest + i) < 123)
                *(dest + i) = *(dest + i) - 32;
        }
    }

    return 0;
}

int fat32_getDirEntry(fat32_driver_info* part, char* filename, direntry** entry)
{
    int result = 0;
    unsigned int filesFound = 0;
    direntry* entries = fat32_listDirectory(part, part->root_dir_sector, &filesFound);

    ReturnOnFailure((filesFound < 1), "Failed to read root directory.\n");

    char* fatFilename = (char*)palloc(11);
    ReturnOnFailureF((result = fat32_translateTo83FatName(filename, fatFilename)), "Failed to translate '%s' to 8.3 format.\n", filename);
   
    direntry* file = 0;
    unsigned int i;
    unsigned int max = filesFound + 1;
    for (i = 0; i < max; i++)
    {
        // Only search for files for now, no directory support
        if (entries[i].attribute.bits.directory)
            continue;

        if (my_strcmp_s((char*)entries[i].name, 11, fatFilename) == 0)// || 
            //my_strcmp((char*)entries[i].longName, filename) == 1)
        {
            file = &entries[i];
            break; // Found it!
        }
    }

    ReturnOnFailureF((file == 0), "Couldn't find file with name '%s'\n", filename);

    printf("Get entry, size: %d\n", file->size);

    // Allocate a new entry with the data so that it isn't tied to the arg
    *entry = (direntry*)pcalloc(sizeof(direntry), 1);
    my_memcpy(*entry, file, sizeof(direntry));

    return 0;

fExit:
    return -1;
}

static int fat32_unicode16ToLongName(fat32_lfe* dest, unsigned short* src, unsigned int srcLength)
{
    // TODO: Isn't this just a straight up copy of the memory? :S
    unsigned int i;
    for (i = 0; i < srcLength; i++)
    {
        if (src[i] == 0)
        {
            dest->length = i;
            dest->name[i] = 0;
            return 0; // Done reading, success!
        }

        dest->name[i] = (char)src[i];
    }

    dest->length = srcLength;

    return i;
}

static direntry* fat32_listDirectory(fat32_driver_info* part, unsigned int firstSector, unsigned int* pFilesFound)
{
    direntry* entries = (direntry*)pcalloc(sizeof(direntry), 19);
    fat32_lfe* entries_long = (fat32_lfe*)pcalloc(sizeof(fat32_lfe), 19);

    unsigned int lfn_count = 0;
    unsigned int last_lfn = 0;
    unsigned int file_index = 0;

    unsigned int doneReading = 0;

    unsigned int blockOffset = 0;
    while (!doneReading)
    {
        // Read a block (NOTE: This fails miserably on multi-cluster directories)
        unsigned int blockToRead = firstSector + blockOffset;
        if (part->basic.device->operation(OpRead, &blockToRead, part->basic.device->buffer) != S_OK)
        {
            printf("Failed to read sector %d\n", blockToRead);
            break;
        }
        
        unsigned char* buf = part->basic.device->buffer;

        // 16 entries per block
        unsigned int i;
        for (i = 0; i < 16; i++)
        {
            unsigned int currentEntryOffset = i * 32; // Entries are 32 byte
            if (buf[0 + currentEntryOffset] == 0)
            {
                doneReading = 1;
                break;
            }
            else if (buf[0 + currentEntryOffset] == 0xE5) // Free entry
                continue;

            if (buf[11 + currentEntryOffset] == LONG_FILE_ENTRY_SIG)
            {
                // Long entry
                unsigned char lfn_index = ((buf[0 + currentEntryOffset] - 1) & ~0x40); // Just ignore the "last lfn entry" flag

                unsigned char* tmp = (unsigned char*)palloc(sizeof(short)* 26);
                my_memcpy(&tmp[0], &buf[1 + currentEntryOffset], 10);
                my_memcpy(&tmp[0 + 10], &buf[14 + currentEntryOffset], 12);
                my_memcpy(&tmp[0 + 10 + 12], &buf[28 + currentEntryOffset], 4);

                fat32_unicode16ToLongName(&entries_long[lfn_index], (unsigned short*)tmp, 13);

                phree(tmp);

                lfn_count++;

                last_lfn = 1;
            }
            else
            {
                // Short entry
                if (last_lfn)
                {
                    char lfn[255];
                    unsigned int offset = 0;
                    unsigned int lfn_index = 0;

                    // Copy concatenate all the long name entries into a string
                    for (lfn_index = 0; lfn_index < lfn_count; lfn_index++)
                    {
                        my_memcpy(&lfn[offset], entries_long[lfn_index].name, entries_long[lfn_index].length);
                        offset += entries_long[lfn_index].length;
                    }

                    my_memcpy(&entries[file_index].longName, lfn, offset + 1);
                    entries[file_index].longName[offset] = 0; // null terminated
                    entries[file_index].hasLongName = 1;

                    last_lfn = 0;
                }

                // Copy the entire short entry to the dir entry
                my_memcpy(&entries[file_index], &buf[0 + currentEntryOffset], 32);

                file_index++;
            }
        }

        blockOffset++; // Read next block
    }

    *pFilesFound = file_index - 1; // 0 based

    // No longer need the long entries, their info has been copied to entries
    phree(entries_long);

    return entries;
}

static char* fat32_dateToString(short date)
{
    unsigned short year, month, day;

    char* res = (char*)palloc(11); // yyyy-mm-dd\0 format

    year = 1980 + ((date & 0xFE00) >> 9); // Year is stored in 15-9 (0 = 1980)
    month = (date & 0xF0) >> 4; // Month is stored in 8-5 (1 - 12)
    day = (date & 0x1F);

    itoa(year, &res[0]);
    res[4] = '-';
    itoa(month, &res[5]);

    // Ugly special handling for now until I add capabilities for itoa to padd with n zeroes
    unsigned int offset = 0;
    if (month < 10)
        offset = 6;
    else
        offset = 7;

    res[offset] = '-';
    offset++;

    itoa(day, &res[offset]);

    return res;
}

static char* fat32_timeToString(short time)
{
    unsigned short hour, minute, second;
    char* res = (char*)palloc(9); // HH:mm:ss (length: 9)

    hour = ((time & 0xF800) >> 11); // 15-11 (0-23)
    minute = ((time & 0x7FF) >> 5); // 10-5 (0-59)
    second = (time & 0x1F) * 2; // 4-0 (0-29, seconds / 2)

    itoa(hour, res);

    unsigned int offset;
    if (hour < 9)
        offset = 1;
    else
        offset = 2;

    res[offset] = ':';
    offset++;

    itoa(minute, &res[offset]);

    if (minute < 9)
        offset += 1;
    else
        offset += 2;

    res[offset] = ':';
    offset++;

    itoa(second, &res[offset]);

    return res;
}

static int fat32_driver_read(fat32_driver_info* info, long handle, char* buf, long long bytesToRead)
{
    printf("Fat32_driver_read(info, %d, 0x%h, %d\n", handle, &buf, bytesToRead);

    int result = 0;
    unsigned int openFileIndex = (handle & 0xFF);

    if (openFileIndex > 4)
        return -1; // Invalid file index

    char* argBuf = (char*)buf;
    direntry_open* file = 0; // HOW DO WE GET A HOLD OF THE OPEN INSTANCE? &partition->openFiles[openFileIndex];

    // Figure out how many bytes to read
    unsigned long int bytesLeftInFile = file->entry->size - file->offset;
    if (bytesLeftInFile < bytesToRead)
        bytesToRead = bytesLeftInFile;

    unsigned int fileStartCluster = file->entry->first_cluster_high << 16 | file->entry->first_cluster_low;
    unsigned int fileFirstSector = info->root_dir_sector + ((fileStartCluster - 2) * info->boot_sector.sectors_per_cluster);
    
    //unsigned int curStreamSector = file->streamPosition / 512;
    unsigned int sectorsToRead = bytesToRead / 512;
    sectorsToRead += bytesToRead % 512 != 0 ? 1 : 0;

    unsigned int totalBytesRead = 0;
    unsigned int i;
    if (sectorsToRead < info->boot_sector.sectors_per_cluster)
    {
        // Oh joy! We only have to read one cluster
        for (i = 0; i < sectorsToRead; i++)
        {
            unsigned int sectorToRead = fileFirstSector + i;
            
            ReturnOnFailureF(result = info->basic.device->operation(OpRead, &sectorToRead, info->basic.device->buffer),
                "Failed to read sector %d for file with handle %d.\n", sectorToRead, handle);

            unsigned int bytesRead = bytesToRead > 512 ? 512 : bytesToRead;

            my_memcpy((argBuf + totalBytesRead), info->basic.device->buffer, bytesRead);

            bytesToRead -= bytesRead;
            totalBytesRead += bytesRead;
        }
    }
    else
    {
        
        // THe file is spread across multiple clusters - we need the FAT for this one
        unsigned int clustersToRead = sectorsToRead / info->boot_sector.sectors_per_cluster;
        clustersToRead += sectorsToRead % info->boot_sector.sectors_per_cluster != 0 ? 1 : 0;

        unsigned int currentCluster = fileStartCluster;
        for (i = 0; i < clustersToRead; i++)
        {
            // Read the cluster (8 * 512 byte blocks, 4096 bytes)
            unsigned int j;
            for (j = 0; j < info->boot_sector.sectors_per_cluster; j++)
            {
                
                unsigned int lba = (info->root_dir_sector + ((currentCluster - 2) * info->boot_sector.sectors_per_cluster)) + j;
                
                ReturnOnFailureF(result = info->basic.device->operation(OpRead, &lba, info->basic.device->buffer), 
                    "Failed to read sector %d for file with handle %d. Cluster: %d\n", lba, handle, currentCluster);

                unsigned int bytesRead = bytesToRead > 512 ? 512 : bytesToRead;

                my_memcpy(argBuf + totalBytesRead, info->basic.device->buffer, bytesRead);

                bytesToRead -= bytesRead;
                totalBytesRead += bytesRead;
            }

            unsigned int fatEntry = fat32_getFatEntry(info, currentCluster);
            if (fatEntry == -1)
            {
                printf("Failed to retrieve FAT entry for cluster. :-(");
                break;
            }

            // Ignore the 4 high bits
            fatEntry &= 0x0FFFFFFF;

            if (fatEntry >= 0x0FFFFFF8)
                break; // There are no more clusters to read! (this shouldn't happen?)
            else if (fatEntry == 0x0FFFFFF7)
                break; // Bad cluster :-( Report to user?

            currentCluster = fatEntry;
        }
    }

fExit:
    return result;
}

static int fat32_driver_write()
{
    printf("Fat32 driver does not support writing yet...\n");

    return -1;
}

static void read_boot_sector(fat_boot_sector* boot_sector, char* buffer)
{
    // I'm currently doing this in a really nasty way at the moment because
    // I want the partition name for to be null terminated, and also because I'm
    // Not storing all information in the boot sector, like the bootloader code
    // TODO: Make the fat_boot_sector struct perfectly match what's on disk
    // So that this function can be replaced with a single call to to copy the memory
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
    read_boot_sector(&info->boot_sector, (char*)device->buffer);
    
    info->first_sector = byte_to_int(pInfo->lba_begin);

    info->root_dir_sector = info->first_sector + info->boot_sector.num_reserved_sectors + 
        (info->boot_sector.num_fats * info->boot_sector.sectors_per_fat);
    
    // Assign the return value
    *driver_info = (fs_driver_info*)info;
   
    return 0;
}

int fat32_driver_operation(fs_driver_info* info, fs_op operation, void* arg1, void* arg2, void* arg3)
{
    switch (operation)
    {
    case fs_op_read:
        return fat32_driver_read((fat32_driver_info*)info, (unsigned int)arg1, arg2, (long long)arg3);
    case fs_op_open:
        return fat32_getDirEntry((fat32_driver_info*)info, (char*)arg1, (direntry**)arg2);
    default:
        // Not supported - Probably write?
        return -1;
    }
}
