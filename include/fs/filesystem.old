// #include "hardware/device/blockDevice.h"

// #define BOOTABLE_PARTITION 0x80s

// #ifndef FILESYSTEM_H
// #define FILESYSTEM_H

// typedef enum{
//     UnknownFsType = 0x00,
//     Fat12 = 0x01,
//     XenixRoot = 0x02,
//     XenixUser = 0x03,
//     FAT16LessThan32Mb = 0x04,
//     ExtendedMsDos = 0x05,
//     FAT16Over32Mb = 0x06,
//     Ntfs = 0x07, // Can also be IFS or exFAT
//     Fat16Ext = 0x06,
//     FAT32WithCHS = 0x0B, // Partitions up to 2047MB
//     FAT32XWithLba = 0x0C, // Lba extension
//     Fat16WithLba = 0xE,
//     Fat32WithLba3 = 0xF,
//     Linux = 0x93,
//     LinuxSwap = 0x42,
//     LinuxNative = 0x43,
//     LinuxSwap2 = 0x82,
//     LinuxNative2 = 0x83,
//     LinuxExtended = 0x85,
//     OsxBoot = 0xAB
// } FsType;

// typedef enum {
//     FsFileClosed = 0,
//     FsOpenRead = 2,
//     FsOpenWrite = 4
// } FileSystemOpenMode;

// typedef enum {
//     FsSeekBegin = 0,
//     FsSeekCurrent = 2,
//     FsSeekEnd = 4
// } FsSeekOrigin;

// typedef union{
//     unsigned char raw;
//     struct {
//         unsigned char readOnly : 1;
//         unsigned char hidden : 1;
//         unsigned char systemFile : 1;
//         unsigned char volumeId : 1; // This attribute is also used to indicate that the file entry is a Long file name entry
//         unsigned char directory : 1; // Is a Sub directory (32-byte records)
//         unsigned char archive : 1; // Has changed since backup
//         unsigned char unused : 1; // 0
//         unsigned char unused2 : 1; // 0
//     } bits;
// } FileAttribute;

// typedef struct {
//     unsigned char name[11]; // Offset; 0x00 - (11 bytes)
//     FileAttribute attribute; // Offset: 0x0B
//     unsigned char reserved; // Reserved for Windows NT (Supposedly this tells us about the casing)
//     unsigned char creationTimeInTenths; // Creation time in tenths of a second. Note: Only 24 bits used
//     unsigned short createTime; // 5 bits Hour, 6 bits minutes, 5 bits seconds
//     unsigned short createDate; // 7 bits year, 4 bits month, 5 bits day
//     unsigned short lastAccessDate; // 
//     unsigned short firstClusterHigh; // High 16 bits is first cluster number
//     unsigned short lastModifiedTime;
//     unsigned short lastModifiedDate;
//     unsigned short firstClusterLow;  // Low 16 bits is the first cluster number
//     unsigned int size; // In bytes
//     unsigned char hasLongName;
//     unsigned char longName[256];
// } DirEntry;

// typedef struct {
//     long int streamPosition;
//     FileSystemOpenMode mode;
//     unsigned char filename[84];
//     DirEntry* file;
// } FileStatus;

// typedef struct {
//     FsType type;
//     void* data; // Pointer to storage of filesystem data
//     BlockDevice* device;
//     unsigned int firstSector;
//     FileStatus openFiles[5];
//     unsigned int partitionId;
//     unsigned int ownerDeviceId;

//     int(*open)(void* part, char* filename, FileSystemOpenMode mode);
//     int(*read)(void* part, void* buffer, unsigned long int bytesToRead, int handle);
//     unsigned int(*write)(void* part, char* filename);
//     int(*close)(void* part, int handle);
// } Partition;

// typedef struct {
//     unsigned char bootFlag;     // 00 = non-bootable, 80 = bootable
//     unsigned char cbsBegin[3];   // Ignore
//     unsigned char type;          // see FsType
//     unsigned char cbsEnd[3];     // Ignore
//     unsigned char lbaBegin[4];   // Logical block address of first block of partition on disk
//     unsigned char numSectors[4]; // Don't care
// } MbrPartitionEntry;

// typedef struct {
//     char bootloader[446];
//     MbrPartitionEntry partitions[4];
//     unsigned short signature;
// } MasterBootRecord;

// typedef struct {
//     unsigned int initialized;
//     MasterBootRecord mbr;
//     Partition partitions[4]; // Note: No support for extended partitions
// } LogicalDevice;

// typedef struct {
//     LogicalDevice devices[2];
// } FileSystem;

// int Fs_Initialize(BlockDevice* device);
// int Fs_Open(char* filename, FileSystemOpenMode mode);
// int Fs_Close(int handle);
// int Fs_Seek(int handle, long int offset, FsSeekOrigin origin);
// int Fs_Tell(int handle);
// int Fs_Read(void* buf, unsigned long int bytesToRead, int handle);

// MbrPartitionEntry* Fs_getFirstActivePartition(MasterBootRecord* record);

// #endif