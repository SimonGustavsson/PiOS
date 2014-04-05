#include "hardware/device/blockDevice.h"
#define BOOTABLE_FLAG 0x80

#define E_UNSUPPORTED -2

#ifndef FS_H
#define FS_H

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
    long long offset;
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
    storage_read = 0,
    storage_write = 2
} storage_device_op;

typedef enum {
    fs_op_read = 0,
    fs_op_open = 2,
    fs_op_write = 4,
    fs_op_close = 8,
    fs_op_seek = 16,
    fs_op_tell = 32,
    fs_op_peek = 64
} fs_op;

// Note: Stupid forward declare because my editor is using clang to
// provide program-time error checking, and it requires definitions
// To be provided the first time they're used
typedef struct fs_driver_info fs_driver_info;
typedef struct fs_driver fs_driver;

// This struct represents an initialized file system driver on a specific
// Block device that has been initialized and can be used by Parition*'s to read
// It is created by a drivers factory function that is registered via a call to fs_register_driver_factory
struct fs_driver_info {
    char* name; // The name of the device on the system used in fopen("hdd/...") f.ex
    unsigned int name_len;
    BlockDevice* device;
    part_info* info;

    int(*operation)(fs_driver_info* info, storage_device_op op, void* arg1, void* arg2);
    // Driver specific info will  be here, but the FS don't care about it
    // This just provides the specific filesystems with a place to store
    // Initialization information
    // The FAT driver for example will return a fat_driver_info struct
    // from it's fs_init() call, containing the VBR and first sector to read
};

typedef struct {
    fs_driver_info* driver;
    char* name;
    unsigned int name_len;
    fs_type type;
    long long size;
    direntry_open* open_dirs;
    unsigned int num_open_dirs;
} partition;

typedef struct {
    int initialized;
    BlockDevice* device;
    long long size;
    partition* partitions[4];
    unsigned int num_partitions;
} storage_device;

typedef struct {
    char bootloader[446];
    part_info partitions[4];
    unsigned short signature;
} mbr_t;

#define MAX_FS_DRIVER_FACTORIES 4
typedef struct {
    storage_device* devices[9];
    unsigned int numDevices;

    int(*factories[MAX_FS_DRIVER_FACTORIES])(BlockDevice* device, part_info* pInfo, fs_driver_info** driver_info);
    unsigned int num_factories;
} file_system;

int fs_initialize(void);

int fs_register_driver_factory(int(*factory)(BlockDevice* device, part_info* pInfo, fs_driver_info** driver_info));
int fs_add_device(BlockDevice*);

#endif