#include "fat32.h"
#include "utilities.h"
#include "emmc.h"
#include "stringutil.h"
#include "terminal.h"

static char gBlock_buf[512];
filesystem* gFs;

unsigned int fat32_read_boot_sector(unsigned int block_number)
{
	if(!EmmcReadBlock(&gBlock_buf[0], 512, block_number))
	{
		printf("fat32 - Failed to read volume id block (%d).\n", block_number);
		return -1;
	}
	
	// Read Partition boot sector ("volume ID")
	char* vi = &gBlock_buf[0];

	memcpy(&gFs->boot_sector.partition_type_name[0], &vi[0x3], 8); 
	gFs->boot_sector.partition_type_name[8] = '\0'; // Make oem name print friendly
	memcpy(&gFs->boot_sector.bytes_per_sector, &vi[0x0B], 2);
	memcpy(&gFs->boot_sector.sectors_per_cluster, &vi[0x0D], 1);
	memcpy(&gFs->boot_sector.num_reserved_sectors, &vi[0x0E], 2);
	memcpy(&gFs->boot_sector.num_fats, &vi[0x10], 1);
	memcpy(&gFs->boot_sector.root_entries, &vi[0x11], 2);
	memcpy(&gFs->boot_sector.small_sectors, &vi[0x13], 2);
	memcpy(&gFs->boot_sector.media_type, &vi[0x15], 1);
	memcpy(&gFs->boot_sector.sectors_per_fat, &vi[0x24], 2);
	memcpy(&gFs->boot_sector.sectors_per_head, &vi[0x18], 2);
	memcpy(&gFs->boot_sector.number_of_heads, &vi[0x1A], 2); // heads per cylinder
	memcpy(&gFs->boot_sector.hidden_sectors, &vi[0x1C], 4);
	memcpy(&gFs->boot_sector.large_sectors, &vi[0x20], 4);
	memcpy(&gFs->boot_sector.large_sectors_per_fat, &vi[0x24], 4);
	memcpy(&gFs->boot_sector.flags, &vi[0x28], 2);
	memcpy(&gFs->boot_sector.version, &vi[0x2A], 2);
	memcpy(&gFs->boot_sector.root_dir_start, &vi[0x2C], 4);
	memcpy(&gFs->boot_sector.info_sector, &vi[0x30], 2);
	memcpy(&gFs->boot_sector.backup_sector, &vi[0x32], 2);

	//printf("fat32 - Root partition: OEM name: '%s'.\n", gFs->boot_sector.partition_type_name);	
	//printf("fat32 - Root partition: Bytes per sector: %d.\n", gFs->boot_sector.bytes_per_sector);
	//printf("fat32 - Root partition: Sectors per cluster: %d.\n", gFs->boot_sector.sectors_per_cluster);
	//printf("fat32 - Root partition: Reserved sectors: %d.\n", gFs->boot_sector.num_reserved_sectors);
	//printf("fat32 - Root partition: There are %d copies of the FAT.\n", gFs->boot_sector.num_fats);
	//printf("fat32 - Root partition: There are %d root entries.\n", gFs->boot_sector.root_entries);
	//printf("fat32 - Root partition: There are %d small sectors.\n", gFs->boot_sector.small_sectors); // If 0, large is used instead
	//printf("fat32 - Root partition: The type of the media is %h.\n", gFs->boot_sector.media_type);
	//printf("fat32 - Root partition: %d sectors per fat.\n", gFs->boot_sector.sectors_per_fat);
	//printf("fat32 - Root partition: %d sectors per head.\n", gFs->boot_sector.sectors_per_head);
	//printf("fat32 - Root partition: %d hidden sectors.\n", gFs->boot_sector.hidden_sectors);
	//printf("fat32 - Root partition: %d large sectors.\n", gFs->boot_sector.large_sectors);
	//printf("fat32 - Root partition: %d large sectors per fat.\n", gFs->boot_sector.large_sectors_per_fat);	
	//printf("fat32 - Root partition: Flags: %d\n", gFs->boot_sector.flags);
	//printf("fat32 - Root partition: Version: %d\n", gFs->boot_sector.version);
	//printf("fat32 - Root partition: Root dir start sector: %d.\n", gFs->boot_sector.root_dir_start);
	//printf("fat32 - Root partition: Info sector: %d.\n", gFs->boot_sector.info_sector);
	//printf("fat32 - Root partition: backup sector: %d.\n", gFs->boot_sector.backup_sector);

	if(gFs->boot_sector.bytes_per_sector != 512)
	{
		printf("fat32 - WARNING: %d bytes per sector in boot_sector, expected 512.\n", gFs->boot_sector.bytes_per_sector);
		return -1;
	}

	if(gFs->boot_sector.num_fats != 2)
	{
		printf("fat32 - Invalid number of fats in volume id %d.\n", gFs->boot_sector.num_fats);
		return -1;
	}

	if(vi[0x1FE] != 0x55 || vi[0x1FF] != 0xAA)
	{
		printf("fat32 - Invalid signature in volume id.\n");
		return -1;
	}

	gFs->boot_sector.root_start_sector = gFs->boot_sector.num_reserved_sectors + (gFs->boot_sector.num_fats * gFs->boot_sector.sectors_per_fat);
	gFs->boot_sector.cluster_start_sector = gFs->boot_sector.root_start_sector + (gFs->boot_sector.root_entries * 32) / gFs->boot_sector.bytes_per_sector;

	if(gFs->boot_sector.cluster_start_sector * 32 % gFs->boot_sector.bytes_per_sector)
		gFs->boot_sector.cluster_start_sector += 1;

	//unsigned int cluster_count = 2 + (gFs->boot_sector.large_sectors - gFs->boot_sector.cluster_start_sector - 2) * gFs->boot_sector.sectors_per_cluster;
	//printf("fat32 - Root start: %d.\n", gFs->boot_sector.root_start_sector);
	//printf("fat32 - Cluster start: %d.\n", gFs->boot_sector.cluster_start_sector);
	//printf("fat32 - Cluster count: %d.\n", cluster_count);

	// TODO: Make sure we properly read the extended FAT32 part of the boot sector (See osdev/FAT)
	
	return 0;
}

unsigned int unicode16_to_asci(long_entry* dest, unsigned short* src, int srcLength)
{
	unsigned int i;
	for(i = 0; i < srcLength; i++)
	{
		if(src[i] == 0)
		{
			dest->length = i;
			dest->name[i] = 0;
			return 1;
		}

		dest->name[i] = (char)src[i];
	}

	dest->length = srcLength;

	return i;
}

unsigned int parse_dir_block(char* buf, int buflen, dir_entry* entries)
{
	int root_dir_entry_index;
	char tmp[26]; // Temporary buffer for storing the long file name when converting
	long_entry long_entries[63];
	unsigned int num_long_entries;
	unsigned int i;
	int last_long;

	last_long = 0;
	root_dir_entry_index = 0;
	num_long_entries = 0;

	for(i = 0; i < buflen; i++)
	{
		if(*buf == 0) break; // no more entries

		// If the entry exists
		if(*buf != 0xE5)
		{
			if(buf[11] == 0x0F) // Long entry
			{
				unsigned char index = buf[0] - 1; // 1 based

				if(index & 0x40)
					index &= ~0x40; // Last long entry for this file

				// Copy name to buffer so it's all in one block
				memcpy(&tmp[0], &buf[1], 10);
				memcpy(&tmp[0 + 10], &buf[14], 12);
				memcpy(&tmp[0 + 10 + 12], &buf[28], 4);

				// Store it as ASCI
				unicode16_to_asci(&long_entries[index], (unsigned short*)tmp, 13);

				num_long_entries++;
				last_long = 1;
			}
			else
			{
				if(last_long)
				{
					// Concatenate all long entries into one block
					unsigned int j;
					unsigned int offset = 0;
					char full_name[255];
					for(j = 0; j < num_long_entries; j++)
					{
						memcpy(&full_name[offset], long_entries[j].name, long_entries[j].length);
					
						offset += long_entries[j].length;
					}

					memcpy(entries[root_dir_entry_index].long_name, full_name, offset + 1);
					
					entries[root_dir_entry_index].has_long_name = 1;

					last_long = 0;
				}

				// Copy the 8.3 data over to a struct for convenient access
				memcpy(&entries[root_dir_entry_index], &buf[0], 32);

				root_dir_entry_index++;
			}
		}

		buf += 32; // Parse next entry
	}

	return root_dir_entry_index;
}

unsigned int fat32_initialize(void) // Pass in device?
{
	printf("fat32 - Initializing...\n");
	
	// Read MBR
	if(!EmmcReadBlock(gBlock_buf, 512, 0))
	{
		printf("fat32 - Failed to read mbr sector 0\n");
		return -1;
	}
	
	// Verify signature
	if(gBlock_buf[510] != 0x55 || gBlock_buf[511] != 0xAA)
	{
		printf("fat32 - Invalid mbr signature.\n");
		return -1; // Invalid mbr signature
	}

	// Save partition information in our global struct
	unsigned int i;
	for(i = 0; i < 4; i++)
		memcpy(&gFs->mbr.partitions[i], &gBlock_buf[446 + (i * 16)], 16);

	// Find first FAT32 partition
	unsigned int part_index;
	for(part_index = 0; part_index < 4; part_index++)
	{
		if(gFs->mbr.partitions[part_index].type == FAT32WithCHS || gFs->mbr.partitions[part_index].type == FAT32XWIthLBA)
			break;

		if(part_index == 3)
		{
			printf("fat32 - Failed to locate first FAT32 partition.\n");
			return -1;
		}
	}

	// Read the boot sector
	unsigned int boot_sector_block = byte_to_int((unsigned char*)&gFs->mbr.partitions[part_index].lba_begin[0]);
	if(boot_sector_block < 1)
	{
		printf("fat32 - Invalid boot sector block number (%d), expected a number higher than 0.\n", boot_sector_block);
		return -1;
	}

	if(fat32_read_boot_sector(boot_sector_block) != 0)
	{
		printf("fat32 - Failed to read boot sector.\n");
		return -1;
	}

	unsigned int root_dir_lba = boot_sector_block + (gFs->boot_sector.sectors_per_fat * gFs->boot_sector.num_fats) + gFs->boot_sector.num_reserved_sectors;
	if(!EmmcReadBlock(gBlock_buf, 512, root_dir_lba))
	{
		printf("Failed to read root dir block %d.\n", root_dir_lba);
		return -1;
	}

	char* rd = (char*)&gBlock_buf[0];

	// Read file table
	printf("Files in root/:\n");
	dir_entry rootDirEntries[50];

	unsigned int entries_found = parse_dir_block(rd, 512, rootDirEntries);

	for(i = 0; i < entries_found; i++)
	{
		dir_entry* e = &rootDirEntries[i];

		if(e->has_long_name)
			printf((char*)e->long_name);
		else
			print((char*)e->name, 8);

		printf(" Attribute: %d.\n", e->attribute.raw);
	}
	
	printf("\nfat32 - Initialize success.\n");

	return 0;
}