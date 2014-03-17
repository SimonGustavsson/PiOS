#include "fs/fat32.h"
#include "util/utilities.h"
#include "hardware/emmc.h"
#include "types/string.h"
#include "terminal.h"
#include "memory.h"

static char gBlock_buf[512];
filesystem* gFs;

unsigned int fat32_read_boot_sector(unsigned int block_number)
{
	if(!Emmc_ReadBlock(&gBlock_buf[0], 512, block_number))
	{
		printf("fat32 - Failed to read volume id block (%d).\n", block_number);
		return -1;
	}
	
	// Read Partition boot sector ("volume ID")
	char* vi = &gBlock_buf[0];

	my_memcpy(&gFs->boot_sector.partition_type_name[0], &vi[0x3], 8); 
	gFs->boot_sector.partition_type_name[8] = '\0'; // Make oem name print friendly
    my_memcpy(&gFs->boot_sector.bytes_per_sector, &vi[0x0B], 2);
    my_memcpy(&gFs->boot_sector.sectors_per_cluster, &vi[0x0D], 1);
    my_memcpy(&gFs->boot_sector.num_reserved_sectors, &vi[0x0E], 2);
    my_memcpy(&gFs->boot_sector.num_fats, &vi[0x10], 1);
    my_memcpy(&gFs->boot_sector.root_entries, &vi[0x11], 2);
    my_memcpy(&gFs->boot_sector.small_sectors, &vi[0x13], 2);
    my_memcpy(&gFs->boot_sector.media_type, &vi[0x15], 1);
    my_memcpy(&gFs->boot_sector.sectors_per_fat, &vi[0x24], 2);
    my_memcpy(&gFs->boot_sector.sectors_per_head, &vi[0x18], 2);
    my_memcpy(&gFs->boot_sector.number_of_heads, &vi[0x1A], 2); // heads per cylinder
    my_memcpy(&gFs->boot_sector.hidden_sectors, &vi[0x1C], 4);
    my_memcpy(&gFs->boot_sector.large_sectors, &vi[0x20], 4);
    my_memcpy(&gFs->boot_sector.large_sectors_per_fat, &vi[0x24], 4);
    my_memcpy(&gFs->boot_sector.flags, &vi[0x28], 2);
    my_memcpy(&gFs->boot_sector.version, &vi[0x2A], 2);
    my_memcpy(&gFs->boot_sector.root_dir_start, &vi[0x2C], 4);
    my_memcpy(&gFs->boot_sector.info_sector, &vi[0x30], 2);
    my_memcpy(&gFs->boot_sector.backup_sector, &vi[0x32], 2);

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

	gFs->boot_sector.root_start_sector = gFs->mbr.boot_sector_block + gFs->boot_sector.num_reserved_sectors + 
		(gFs->boot_sector.num_fats * gFs->boot_sector.sectors_per_fat);

	//unsigned int cluster_count = 2 + (gFs->boot_sector.large_sectors - gFs->boot_sector.root_start_sector - 2) * gFs->boot_sector.sectors_per_cluster;
	//printf("fat32 - Root start: %d.\n", gFs->boot_sector.root_start_sector);
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

//
//unsigned int parse_dir_block(char* buf, int buflen, dir_entry* entries)
//{
//	int root_dir_entry_index;
//	char tmp[26]; // Temporary buffer for storing the long file name when converting
//	long_entry long_entries[63];
//	unsigned int num_long_entries;
//	unsigned int i;
//	int last_long;
//
//	last_long = 0;
//	root_dir_entry_index = 0;
//	num_long_entries = 0;
//
//	for(i = 0; i < buflen; i++)
//	{
//		if(*buf == 0) break; // no more entries
//
//		// If the entry exists
//		if(*buf != 0xE5)
//		{
//			if(buf[11] == 0x0F) // Long entry
//			{
//				unsigned char index = buf[0] - 1; // 1 based
//
//				if(index & 0x40)
//					index &= ~0x40; // Last long entry for this file
//
//				// Copy name to buffer so it's all in one block
//				memcpy(&tmp[0], &buf[1], 10);
//				memcpy(&tmp[0 + 10], &buf[14], 12);
//				memcpy(&tmp[0 + 10 + 12], &buf[28], 4);
//
//				// Store it as ASCI
//				unicode16_to_asci(&long_entries[index], (unsigned short*)tmp, 13);
//
//				num_long_entries++;
//				last_long = 1;
//			}
//			else
//			{
//				if(last_long)
//				{
//					// Concatenate all long entries into one block
//					unsigned int j;
//					unsigned int offset = 0;
//					char full_name[255];
//					for(j = 0; j < num_long_entries; j++)
//					{
//						memcpy(&full_name[offset], long_entries[j].name, long_entries[j].length);
//					
//						offset += long_entries[j].length;
//					}
//
//					memcpy(entries[root_dir_entry_index].long_name, full_name, offset + 1);
//					
//					entries[root_dir_entry_index].has_long_name = 1;
//
//					last_long = 0;
//				}
//
//				// Copy the 8.3 data over to a struct for convenient access
//				memcpy(&entries[root_dir_entry_index], &buf[0], 32);
//
//				root_dir_entry_index++;
//			}
//		}
//
//		buf += 32; // Parse next entry
//	}
//
//	return root_dir_entry_index;
//}

static unsigned int parse_directory_block(char* buf, int buflen, dir_entry* entries, unsigned int max_entries)
{
	unsigned int i;

	long_entry long_entries[19]; // Max 19 entries @ 13 asci chars each = 247byte max filename length
	unsigned int lfn_count = 0;
	unsigned int last_lfn = 0;
	unsigned int file_index = 0;

	for(i = 0; i < buflen; i++)
	{
		if(buf[0] == 0) break; // Done reading
		if(*buf == 0xE5) continue; // Doesn't exist
		
		if(LONG_FILE_ENTRY_SIG == buf[11])
		{
			unsigned char lfn_index = ((buf[0] - 1) & ~0x40); // Just ignore the "last lfn entry" flag

			// Read the file name into a block
			unsigned short tmp[26];
			my_memcpy(&tmp[0], &buf[1], 10);
            my_memcpy(&tmp[0 + 10], &buf[14], 12);
            my_memcpy(&tmp[0 + 10 + 12], &buf[28], 4);

			// Convert from Unicode16 and store it as ASCI
			unicode16_to_asci(&long_entries[lfn_index], tmp, 13);

			lfn_count++;

			last_lfn = 1;
		}
		else
		{
			if(last_lfn)
			{
				// Concatenate all long file entry values into the full name
				char lfn[255];
				unsigned int offset = 0;
				unsigned int lfn_index;
				for(lfn_index = 0; lfn_index < lfn_count; lfn_index++)
				{
					my_memcpy(&lfn[offset], long_entries[lfn_index].name, long_entries[lfn_index].length);

					offset += long_entries[lfn_index].length;
				}

				// Store the name in the file entry
				my_memcpy(entries[file_index].long_name, lfn, offset + 1); // Null terminated
				entries[file_index].has_long_name = 1;
				
				last_lfn = 0;
			}

			// Copy 8.3 data to the file entry
			my_memcpy(&entries[file_index], &buf[0], 32);

			file_index++;
		}

		buf += 32; // Advance to the next entry
	}

	return file_index; // Was 0 indexed
}

unsigned int read_dir_at_cluster(char* buf, unsigned int buflen, dir_entry* entries, unsigned int max_entries, unsigned int cluster)
{
	unsigned int cluster_lba = gFs->boot_sector.root_start_sector + (cluster * gFs->boot_sector.sectors_per_cluster);

	unsigned int cur_block;
	unsigned int max_block = gFs->boot_sector.sectors_per_cluster;
	unsigned int files_read = 0;
	dir_entry* entries_ptr;

	for(cur_block = 0; cur_block < max_block; cur_block++)
	{
		unsigned int block_lba = cluster_lba + cur_block;
		
		if(!Emmc_ReadBlock(buf, buflen, block_lba))
		{
			printf("fat32 - Could not read block: %d of cluster: %d.\n", block_lba, cluster);

			return -1;
		}

		unsigned int num_files = parse_directory_block(buf, buflen, entries, max_entries - files_read);

		if(num_files < 1)
		{
			// Couldn't read any files, we must be at the end of the directory
			break;
		}
		
		files_read += num_files;

		// Increment the entries ptr so that the next call places the entries after the ones we just found
		entries_ptr += num_files;
	}

	return files_read;
}

unsigned int fat32_initialize(void) // Pass in device?
{
	printf("fat32 - Initializing...\n");
	
	// Read MBR
	if(!Emmc_ReadBlock(gBlock_buf, 512, 0))
	{
		printf("fat32 - Failed to read mbr sector.\n");
		return -1;
	}

    //return 0;

	// Verify signature
	if(gBlock_buf[510] != 0x55 || gBlock_buf[511] != 0xAA)
	{
		printf("fat32 - Invalid mbr signature, dumping first block:\n");

		unsigned int index;
		unsigned int i;
		for (i = 0; i < 16; i++)
		{
			unsigned int j;
			for (j = 0; j < 32; j++)
			{
				index = (i * 32) + j;

				if (gBlock_buf[index] == 0)
					printf("0");
				else if (gBlock_buf[index] > 126)
					printf(".");
				else
					printf("%h ", gBlock_buf[(i * 32) + j]);
			}
			printf("\n");
		}

		return -1; // Invalid mbr signature
	}

    gFs = (filesystem*)palloc(sizeof(filesystem));

	// Save partition information in our global struct
	unsigned int i;
	for(i = 0; i < 4; i++)
		my_memcpy(&gFs->mbr.partitions[i], &gBlock_buf[446 + (i * 16)], 16);

    return 0;

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
	gFs->mbr.boot_sector_block = byte_to_int((unsigned char*)&gFs->mbr.partitions[part_index].lba_begin[0]);
	if(gFs->mbr.boot_sector_block < 1)
	{
		printf("fat32 - Invalid boot sector block number (%d), expected a number higher than 0.\n", gFs->mbr.boot_sector_block);
		return -1;
	}

	printf("boot sector block: %d.\n", gFs->mbr.boot_sector_block);
	if(fat32_read_boot_sector(gFs->mbr.boot_sector_block) != 0)
	{
		printf("fat32 - Failed to read boot sector.\n");
		return -1;
	}
		
	// Read root directory
	dir_entry rootDirEntries[50];

	unsigned int root_entries = read_dir_at_cluster(gBlock_buf, 512, rootDirEntries, 50, gFs->boot_sector.root_dir_start - 2);
	
	printf("%d files in root/:\n", root_entries);
	for(i = 0; i < root_entries; i++)
	{
		dir_entry* e = &rootDirEntries[i];

		if(e->attribute.bits.directory)
		{
            printf("[");
		}

		if(e->has_long_name)
			printf((char*)e->long_name);
		else
			printf((char*)e->name);
		
		if(e->attribute.bits.directory)
		{
            printf("]");
		}

		printf(" - Cluster: %d.\n", e->first_cluster_low);
	}
	
	printf("\nfat32 - Initialize success.\n");

	return 0;
}
