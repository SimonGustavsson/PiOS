
#define BOOTABLE_FLAG 0x80

typedef enum{
    UnknownFsType = 0x00,
    Fat12 = 0x01,
    XenixRoot = 0x02,
    XenixUser = 0x03,
    FAT16LessThan32Mb = 0x04,
    ExtendedMsDos = 0x05,
    FAT16Over32Mb = 0x06,
    Ntfs = 0x07, // Can also be IFS or exFAT
    Fat16Ext = 0x06,
    FAT32WithCHS = 0x0B, // Partitions up to 2047MB
    FAT32XWithLba = 0x0C, // Lba extension
    Fat16WithLba = 0xE,
    Fat32WithLba3 = 0xF,
    Linux = 0x93,
    LinuxSwap = 0x42,
    LinuxNative = 0x43,
    LinuxSwap2 = 0x82,
    LinuxNative2 = 0x83,
    LinuxExtended = 0x85,
    OsxBoot = 0xAB
} fs_type;

typedef enum {
    seek_begin = 0,
    seek_relative = 2,
    seek_end = 4
} seek_origin;

typedef enum {
    file_closed = 0,
    file_read = 2,
    file_write = 4
} file_mode;

typedef enum {
    unknown = 0,
    file = 2,
    directory = 4,
    device = 8
} direntry_type;

typedef struct {
    unsigned int it;
    char* name;
    unsigned int name_len;
} direntry;

typedef struct {
    direntry* entry;
    long long offet;
    file_mode mode;
} direntry_open;

typedef struct {
    unsigned char bootable;       // BOOTABLE_FLAG
    unsigned char cbs_begin[3];   // Don't use
    unsigned char type;           // see fs_type
    unsigned char cbs_end[3];     // Don't use
    unsigned char lba_begin[4];   //
    unsigned char num_sectors[4]; // Size?
} part_info;

typedef enum {
    unknown_device = 0,
    storage_device = 2,
    removable_device = 4
} storage_device_type;

typedef enum {
    storage_read = 0,
    storage_open = 2,
    storage_write = 4,
    storage_close = 8,
    storage_seek = 16,
    storage_tell = 32
} storage_device_op;

typedef struct {
    char device; // Placeholder for block device struct
    char* name;
    unsigned int name_len;
    part_info* info;
    fs_type type;
    long long size;
    direntry_open* open_dirs;
    unsigned int num_open_dirs;

    int(*operation)(storage_device_op, void* arg1, void* arg2);
} partition;

typedef struct {
    storage_device_type type;
    long long size;
    char* driver_name;
    unsigned int driver_name_len;
    partition* partitions;
    unsigned int num_partitions;
} storage_device;
