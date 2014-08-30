#include "memory.h"
#include "fs/fs.h"
#include "util/utilities.h"

#define LONG_FILE_ENTRY_SIG 0x0F

typedef struct {
    unsigned char name[255];
    unsigned int length;
} fat32_lfe;


typedef struct {
    unsigned char jump_instruction[3];     // 0xEB 0x58 0x90
    unsigned char partition_type_name[9];  // MSDOS5.0

    // BIOS Parameter block
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

    // Extended FAT32
    unsigned short flags;                // 0x00 0x00 (dec 0)
    unsigned short version;              // 0x00 0x00 (dec 0)
    unsigned int root_dir_start;         // 0x02 0x00 0x00 0x00 (Cluster where the root directory can be found) (dec 2)
    unsigned short info_sector;          // 0x01 0x00 (File system info sector) (dec 1)
    unsigned short backup_sector;        // 0x06 0x00 (Sector of FAT backup of this sector) (dec 6)
    
    // NOTE: Omitted some information here such as volume ID

    // NOTE: Omitted bootstrapper code and signature

    unsigned int root_start_sector;
} fat_boot_sector;

typedef struct {
    fs_driver_info basic;

    fat_boot_sector boot_sector; // This is really a struct, TODO
    unsigned int first_sector;
    unsigned int root_dir_sector;
} fat32_driver_info;

// Public functions
int fat32_driver_factory(BlockDevice* device, part_info* pInfo, fs_driver_info** driver_info);
int fat32_driver_operation(fs_driver_info* info, fs_op operation, void* arg1, void* arg2, void* arg3);
