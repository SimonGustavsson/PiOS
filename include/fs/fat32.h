#include "fs/filesystem.h"

#ifndef FAT32_H
#define FAT32_H

#define FAT32
#define LONG_FILE_ENTRY_SIGNATURE 0x0

typedef struct {
    unsigned int signature; // Offset: 0x0 - Should always be 0x41615252?
    unsigned int signature2; // Offset: 0x1E4 - Should always be 0x61417272?
    unsigned int free_clusters; // Offset: 0x1E8 - -1 if unknown
    unsigned int most_recent_allocated_cluster; // Offset: 0x1EC -
    unsigned int end_of_sector_marker; // Offset: 0x1FE - Always 0xAA55
} Fat32InfoSector;

typedef struct {
    unsigned char jump_instruction[3];     // 0xEB 0x58 0x90
    unsigned char partition_type_name[9];  // MSDOS5.0
    // Bios parameters
    unsigned short bytes_per_sector;     // 0x00 0x02 (dec 2)
    unsigned char sectors_per_cluster;   // 0x8       (dec 8)
    unsigned short num_reserved_sectors; // 0xEC 0x04 (dec 1260) (Swapped)
    unsigned char num_fats;              // 0x02      (dec 2)
    unsigned short root_entries;         // 0x00 0x00 (dec 0)
    unsigned short small_sectors;        // 0x00 0x00  (dec 0)
    unsigned char media_type;            // F8 (Harddisk)
    unsigned short sectors_per_fat;      // 0x00 0x00 (dec 0)
    unsigned short sectors_per_head;    // 0x3F 0x00 (dec 16128) (63 swapped)
    unsigned short number_of_heads;      // 0xFF 0x00 (dec 65280) (255 swapped)
    unsigned int hidden_sectors;         // 0x80 0x00 0x00 0x00 (dec 2147483648) (Swapped dec 128)
    unsigned int large_sectors;          // 0x80 0x67 0x76 0x00 (dec 2154264064) (Swapped dec 7759744) 
    unsigned int large_sectors_per_fat;  // 0x8A 0x1D (dec 35357) (7562 swapped)

    unsigned short flags;                // 0x00 0x00 (dec 0)
    unsigned short version;              // 0x00 0x00 (dec 0)
    unsigned int root_dir_start;         // 0x02 0x00 0x00 0x00 (Cluster where the root directory can be found) (dec 2)
    unsigned short info_sector;          // 0x01 0x00 (File system info sector) (dec 1)
    unsigned short backup_sector;        // 0x06 0x00 (Sector of FAT backup of this sector) (dec 6)

    // (Omitted bootstrapper code and signature)

    unsigned int root_start_sector;
} Fat32BootSector;

typedef struct {
    unsigned char name[255];
    unsigned int length;
} Fat32Lfe; // Long filename entry

typedef struct {
    Fat32BootSector* vbr;
    unsigned int rootDirectorySector;
    unsigned int* fat0; // Varying size depending on disk size
    unsigned int* fat1; // Varying size depending on disk size
} Fat32Disk;

// Filesystem functions
unsigned int Fat32_Initialize(Partition* partition);
int Fat32_Open(void* part, char* filename, FileSystemOpenMode mode);
int Fat32_Close(void* part, int handle);
int Fat32_Seek(void* part, int handle, long int offset, FsSeekOrigin origin);
int Fat32_Tell(void* part, int handle);
int Fat32_Read(void* part, void* buf, unsigned long int bytesToRead, int handle);
unsigned int Fat32_ReadBlock(Partition* part, char* buffer, unsigned int blockNumber);

// Forward declarations of 'private' functions
DirEntry* Fat32_listDirectory(Partition* part, unsigned int firstSector, unsigned int* pFilesFound);
Fat32BootSector* Fat32_parseVirtualBootRecord(unsigned char* buf, unsigned int buflen);
unsigned int Fat32_unicode16ToLongName(Fat32Lfe* dest, unsigned short* src, unsigned int srcLength);
char* Fat32_dateToString(short date);
char* Fat32_timeToString(short time);

#endif