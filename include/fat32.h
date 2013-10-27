typedef enum{
	UnknownFsType = 0x00,
	Fat12 = 0x01,
	FAT16LessThan32Mb = 0x04,
	ExtendedMsDos = 0x05,
	Fat16Ext = 0x06,
	FAT32WithCHS = 0x0B, // Partitions up to 2048GB) MB?
	FAT32XWIthLBA = 0x0C, // Lba extension
	Fat32WithLba2 = 0xE,
	Fat32WithLba3 = 0xF,
	Linux = 0x93
} fs_type;

typedef struct {
	char boot_flag; // 00 = Non-bootable, 80 = Bootable
	char chs_begin[3]; // Can be ignored? 
	char type; // see fs_type
	char chs_end[3]; // Can be ignored?
	char lba_begin[4]; // Address where partition starts here on disk ( Number of Sectors Betweenthe MBR and the First Sector in the Partition ?)
	char numbers_of_sectors[4]; // FAT32 fs includes this info, so no need to read
} partition_entry;

typedef struct {
	//unsigned char boot_code[446]; // Includes the bootcode (irrelevant on the pi)
	partition_entry partitions[4];
	unsigned char signature[2]; // Should always be 0x55 0xAA
} mbr;

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

	// Calculated offsets
	unsigned int root_start_sector;
	unsigned int cluster_start_sector;
} fat32_boot_sector;

typedef struct {
	mbr mbr;
	fat32_boot_sector boot_sector;

	unsigned int fat_begin_lba;// lba_begin + num_reserved_sectors
	unsigned int cluster_begin_lba; // lba_begin + num_reserved_sectors + (num_fats * sectors_per_fat)
	unsigned int sectors_per_cluster; // sectors_per_cluster (from Fat32)
	unsigned int root_dir_first_cluster; // root_dir_first_cluster (from Fat32)
} filesystem;

typedef union{
	unsigned char raw;
	struct {
		unsigned int read_only : 1;
		unsigned int hidden : 1;
		unsigned int system_file : 1;
		unsigned int volume_id : 1; // File is volume ID
		unsigned int directory : 1; // Is a Sub directory (32-byte records)
		unsigned int archive : 1; // Has changed since backup
		unsigned int unused : 1; // 0
		unsigned int unused2 : 1; // 0
	} bits;
} file_attribute;

typedef struct {
	unsigned int signature; // Offset: 0x0 - Should always be 0x41615252?
	unsigned int signature2; // Offset: 0x1E4 - Should always be 0x61417272?
	unsigned int free_clusters; // Offset: 0x1E8 - -1 if unknown
	unsigned int most_recent_allocated_cluster; // Offset: 0x1EC -
	unsigned int end_of_sector_marker; // Offset: 0x1FE - Always 0xAA55
} fat32_info_sector;

typedef struct {
	unsigned char name[12]; // Offset; 0x00 - (11 bytes)
	file_attribute attribute; // Offset: 0x0B
	unsigned int created_time; // Note: Only 24 bits used
	unsigned short create_date;
	unsigned short last_access_date; // 
	unsigned short last_modified_time;
	unsigned short last_modified_date;
	unsigned short first_cluster;
	unsigned int size;
} dir_entry;


// NOTE: Only 28 bits used for cluster number, clear upper 4 before reading

unsigned int fat32_initialize(void);
