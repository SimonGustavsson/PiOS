#include "fs/fat32.h"
#include "util/utilities.h"
#include "hardware/emmc.h"
#include "types/string.h"
#include "terminal.h"
#include "memory.h"
#include "types/types.h"
#include "fs/filesystem.h"

unsigned int Fat32_translateTo83FatName(char* filename, char* dest)
{
    char* file = filename + 3; // Skip past x:/

    // Split out extension from file name
    unsigned int nameLength = strlen(file);
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

int Fat32_getDirEntry(Partition* part, char* filename, DirEntry** entry)
{
    int result = 0;
    Fat32Disk* disk = (Fat32Disk*)part->data;

    unsigned int filesFound = 0;
    DirEntry* entries = Fat32_listDirectory(part, disk->rootDirectorySector, &filesFound);

    ReturnOnFailure(filesFound < 1, "Failed to read root directory.\n");

    char* fatFilename = (char*)palloc(11);
    ReturnOnFailureF(result = Fat32_translateTo83FatName(filename, fatFilename), "Failed to translate '%s' to 8.3 format.\n", filename);

    DirEntry* file = 0;
    unsigned int i;
    for (i = 0; i < filesFound + 1; i++)
    {
        // Only search for files for now, no directory support
        if (entries[i].attribute.bits.directory)
            continue;

        if (strcmp((char*)entries[i].name, fatFilename) == 0 ||
            strcmp((char*)entries[i].longName, filename) == 0)
        {
            file = &entries[i];
            break; // Found it!
        }
    }

    ReturnOnFailureF((file == 0), "Couldn't find file with name '%s'\n", filename);

    // Allocate a new entry with the data so that it isn't tied to the arg
    *entry = (DirEntry*)palloc(sizeof(DirEntry));
    my_memcpy(*entry, file, sizeof(DirEntry));

    return 0;

fExit:
    return -1;
}

int Fat32_Open(void* part, char* filename, FileSystemOpenMode mode)
{
    int result = 0;
    Partition* partition = (Partition*)part;

    // Check if it's already open
    FileStatus* file = 0;
    unsigned int i;
    int firstAvailableSlot = -1;
    for (i = 0; i < 5; i++)
    {
        if (partition->openFiles[i].file == 0)
        {
            if (firstAvailableSlot == -1)
                firstAvailableSlot = i;

            continue;
        }

        if (strcmp((char*)partition->openFiles[i].filename, filename) == 0)
        {
            if (mode == FsOpenWrite)
                return -1; // Two people can't write to the same file simultaneously 
            else
            {
                // Bingo! Already open so we don't have to search for it!
                file = &partition->openFiles[i];
                break;
            }
        }
    }

    if (file == 0)
    {
        ReturnOnFailure(firstAvailableSlot < 0, "Can't open more files, please close some handles and try again.\n");

        DirEntry* entry = 0;
        ReturnOnFailureF(result = Fat32_getDirEntry(partition, filename, &entry), "Failed to get dir entry for %s.\n", filename);

        file = &partition->openFiles[firstAvailableSlot];

        file->streamPosition = 0;
        file->file = entry;
        file->mode = mode;
        my_memcpy(file->filename, filename, strlen(filename));
    }

    return (partition->ownerDeviceId << 16) & (partition->partitionId << 8) & (firstAvailableSlot);

fExit:

    return -1;
}

int Fat32_Close(void* part, int handle)
{
    int result = 0;

    ReturnOnFailure(result = handle < 0 || handle > 4, "fat32_close: Invalid handle.\n");

    unsigned int openFileIndex = (handle & 0xFF);
    Partition* partition = (Partition*)part;
    FileStatus* file = &partition->openFiles[openFileIndex];

    phree(file->file);
    file->file = 0;
    file->streamPosition = 0;

    // Zero out the name so that it's nice and clean
    unsigned int i;
    for (i = 0; i < 80; i++)
        file->filename[i] = 0;
    file->mode = FsFileClosed;

fExit:
    return result;
}

int Fat32_Seek(void* part, int handle, long int offset, FsSeekOrigin origin)
{
    int result = 0;

    ReturnOnFailure(result = handle < 0 || handle > 4, "Fat32_Seek: Invalid handle.\n");

    unsigned int openFileIndex = (handle & 0xFF);
    Partition* partition = (Partition*)part;
    FileStatus* file = &partition->openFiles[openFileIndex];

    switch (origin)
    {
    case FsSeekBegin:
        file->streamPosition = offset;
        break;
    case FsSeekCurrent:
        file->streamPosition += offset;
        break;
    case FsSeekEnd:
        file->streamPosition = file->file->size - offset;
        break;
    default:
        result = -1;
        printf("Fat32_Seek: Invalid seek origin\n");
        break;
    }

fExit:
    return result;
}

int Fat32_Tell(void* part, int handle)
{
    unsigned int openFileIndex = (handle & 0xFF);

    if (openFileIndex > 4)
        return -1; // Invalid file index

    Partition* partition = (Partition*)part;
    FileStatus* file = &partition->openFiles[openFileIndex];

    return file->streamPosition;
}

int Fat32_Read(void* part, void* buf, unsigned long int bytesToRead, int handle)
{
    int result = 0;
    unsigned int openFileIndex = (handle & 0xFF);

    if (openFileIndex > 4)
        return -1; // Invalid file index

    char* argBuf = (char*)buf;
    Partition* partition = (Partition*)part;
    FileStatus* file = &partition->openFiles[openFileIndex];

    // Figure out how many bytes to read
    unsigned long int bytesLeftInFile = file->file->size - file->streamPosition;
    if (bytesLeftInFile < bytesToRead)
        bytesToRead = bytesLeftInFile;

    Fat32Disk* disk = (Fat32Disk*)partition->data;

    unsigned int fileStartCluster = file->file->firstClusterHigh << 16 | file->file->firstClusterLow;
    unsigned int fileFirstSector = disk->rootDirectorySector + ((fileStartCluster - 2) * disk->vbr->sectors_per_cluster);

    //unsigned int curStreamSector = file->streamPosition / 512;
    unsigned int sectorsToRead = bytesToRead / 512;
    sectorsToRead += bytesToRead % 512 != 0 ? 1 : 0;

    unsigned int totalBytesRead = 0;
    unsigned int i;
    if (sectorsToRead < disk->vbr->sectors_per_cluster)
    {
        // Oh joy! We only have to read one cluster
        for (i = 0; i < sectorsToRead; i++)
        {
            unsigned int sectorToRead = fileFirstSector + i;

            ReturnOnFailureF(result = partition->device->operation(OpRead, &sectorToRead, partition->device->buffer),
                "Failed to read sector %d for file with handle %d.\n", sectorToRead, handle);

            unsigned int bytesRead = bytesToRead > 512 ? 512 : bytesToRead;

            my_memcpy((argBuf + totalBytesRead), partition->device->buffer, bytesRead);

            bytesToRead -= bytesRead;
            totalBytesRead += bytesRead;
        }
    }
    else
    {
        // THe file is spread across multiple clusters - we need the FAT for this one
        unsigned int clustersToRead = sectorsToRead / disk->vbr->sectors_per_cluster;
        clustersToRead += sectorsToRead % disk->vbr->sectors_per_cluster != 0 ? 1 : 0;

        unsigned int currentCluster = fileStartCluster;
        for (i = 0; i < clustersToRead; i++)
        {
            // Read the cluster (8 * 512 byte blocks, 4096 bytes)
            unsigned int j;
            for (j = 0; j < disk->vbr->sectors_per_cluster; j++)
            {
                unsigned int lba = (disk->rootDirectorySector + ((currentCluster - 2) * disk->vbr->sectors_per_cluster)) + j;

                ReturnOnFailureF(result = partition->device->operation(OpRead, &lba, partition->device->buffer), "Failed to read sector %d for file with handle %d. Cluster: %d\n", lba, handle, currentCluster);

                unsigned int bytesRead = bytesToRead > 512 ? 512 : bytesToRead;

                my_memcpy(argBuf + totalBytesRead, partition->device->buffer, bytesRead);

                bytesToRead -= bytesRead;
                totalBytesRead += bytesRead;
            }

            unsigned int fatEntry = disk->fat0[currentCluster] & 0x0FFFFFFF; // Ignore 4 high bits

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

unsigned int Fat32_Write(void* part, char* filename)
{
    printf("You have to write the code first, dummy! :)");

    return 1;
}

unsigned int Fat32_ReadBlock(Partition* part, char* buffer, unsigned int blockNumber)
{
    //Fat32Disk* disk = (Fat32Disk*)part->data;

    unsigned int result = 0;

    ReturnOnFailure(result = part->device->operation(OpRead, &blockNumber, part->device->buffer), "Failed to read first sector of file.\n");

    // Copy read data to the buffer
    my_memcpy(buffer, part->device->buffer, BLOCK_SIZE);

fExit:
    return result;
}

unsigned int Fat32_Initialize(Partition* partition)
{
    unsigned int result = 0;

    partition->open = &Fat32_Open;
    partition->close = &Fat32_Close;
    partition->read = &Fat32_Read;
    partition->write = &Fat32_Write;

    // Read Volume Boot Record
    ReturnOnFailure(result = partition->device->operation(OpRead, &partition->firstSector, partition->device->buffer), "Failed to read volume boot record.\n");

    Fat32Disk* data = (Fat32Disk*)palloc(sizeof(Fat32Disk));
    data->vbr = Fat32_parseVirtualBootRecord(partition->device->buffer, BLOCK_SIZE);

    partition->data = data; // Store it so we can retrieve the precomputed values later on

    // Calculate start of root directory
    data->rootDirectorySector = partition->firstSector + data->vbr->num_reserved_sectors + (data->vbr->num_fats * data->vbr->sectors_per_fat);

    // Process the allocation tables
    unsigned int fatbegin = partition->firstSector + data->vbr->num_reserved_sectors;

    // Allocate memory for the tables
    data->fat0 = (unsigned int*)palloc(data->vbr->sectors_per_fat * data->vbr->bytes_per_sector);
    data->fat1 = (unsigned int*)palloc(data->vbr->sectors_per_fat * data->vbr->bytes_per_sector);

    // Read the tables from disk
    unsigned int i;
    // Fat 1
    for (i = 0; i < data->vbr->sectors_per_fat; i++)
    {
        // TODO: better error message, include fat+device name etc
        unsigned int sector_to_read = fatbegin + i;
        ReturnOnFailure(result = partition->device->operation(OpRead, &sector_to_read, partition->device->buffer), "Failed to read fat table");

        my_memcpy(data->fat0 + BLOCK_SIZE * i, partition->device->buffer, BLOCK_SIZE);
    }

    // Move to second fat
    fatbegin += data->vbr->sectors_per_fat;

    // Fat 2
    for (i = 0; i < data->vbr->sectors_per_fat; i++)
    {
        // TODO: better error message, include fat+device name etc
        unsigned int sector_to_read = fatbegin + i;
        ReturnOnFailure(result = partition->device->operation(OpRead, &sector_to_read, partition->device->buffer), "Failed to read fat table\n");

        my_memcpy(data->fat1 + BLOCK_SIZE * i, partition->device->buffer, BLOCK_SIZE);
    }

fExit:

    return result;
}

unsigned int fat32_unicode16ToLongName(Fat32Lfe* dest, unsigned short* src, unsigned int srcLength)
{
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

Fat32BootSector* Fat32_parseVirtualBootRecord(unsigned char* buf, unsigned int buflen)
{
    Fat32BootSector* boot_sector = (Fat32BootSector*)palloc(sizeof(Fat32BootSector));

    my_memcpy(&boot_sector->partition_type_name[0], &buf[0x3], 8);
    boot_sector->partition_type_name[8] = '\0'; // Make oem name print friendly
    my_memcpy(&boot_sector->bytes_per_sector, &buf[0x0B], 2);
    my_memcpy(&boot_sector->sectors_per_cluster, &buf[0x0D], 1);
    my_memcpy(&boot_sector->num_reserved_sectors, &buf[0x0E], 2);
    my_memcpy(&boot_sector->num_fats, &buf[0x10], 1);
    my_memcpy(&boot_sector->root_entries, &buf[0x11], 2);
    my_memcpy(&boot_sector->small_sectors, &buf[0x13], 2);
    my_memcpy(&boot_sector->media_type, &buf[0x15], 1);
    my_memcpy(&boot_sector->sectors_per_fat, &buf[0x24], 2);
    my_memcpy(&boot_sector->sectors_per_head, &buf[0x18], 2);
    my_memcpy(&boot_sector->number_of_heads, &buf[0x1A], 2); // heads per cylinder
    my_memcpy(&boot_sector->hidden_sectors, &buf[0x1C], 4);
    my_memcpy(&boot_sector->large_sectors, &buf[0x20], 4);
    my_memcpy(&boot_sector->large_sectors_per_fat, &buf[0x24], 4);
    my_memcpy(&boot_sector->flags, &buf[0x28], 2);
    my_memcpy(&boot_sector->version, &buf[0x2A], 2);
    my_memcpy(&boot_sector->root_dir_start, &buf[0x2C], 4);
    my_memcpy(&boot_sector->info_sector, &buf[0x30], 2);
    my_memcpy(&boot_sector->backup_sector, &buf[0x32], 2);

    // TODO: Add error checking on the read values

    return boot_sector;
}

DirEntry* Fat32_listDirectory(Partition* part, unsigned int firstSector, unsigned int* pFilesFound)
{
    //Fat32Disk* disk = (Fat32Disk*)part->data;

    // Read the first block of the directory
    //int result = 0;

    DirEntry* entries = (DirEntry*)palloc(sizeof(DirEntry)* 19);
    Fat32Lfe* entries_long = (Fat32Lfe*)palloc(sizeof(Fat32Lfe)* 19);

    unsigned int lfn_count = 0;
    unsigned int last_lfn = 0;
    unsigned int file_index = 0;

    unsigned int doneReading = 0;

    unsigned int blockOffset = 0;
    while (!doneReading)
    {
        // Read a block (NOTE: This fails miserably on multi-cluster directories)
        unsigned int blockToRead = firstSector + blockOffset;
        part->device->operation(OpRead, &blockToRead, part->device->buffer);

        unsigned char* buf = part->device->buffer;

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

            if (buf[11 + currentEntryOffset] == LONG_FILE_ENTRY_SIGNATURE)
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

    return entries;
}

char* Fat32_dateToString(short date)
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

char* Fat32_timeToString(short time)
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