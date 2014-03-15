#include "memory.h"
#include "util/utilities.h"
#include "types/string.h"
#include "types/types.h"

static unsigned char* gBitmap = (unsigned char*)(0xA00000 + 0x8000); // 10 MB From the kernel at 0x8000
static unsigned char* gMemory = (unsigned char*)((0xA00000 + 0x8000) + (MAX_ALLOCATED_SLICES / 8)); // Put after the bitmap (which size i 0x1900000)

void DataMemoryBarrier(void)
{
	asm volatile ("mcr p15, #0, %[zero], c7, c10, #5" : : [zero] "r" (0) );
}

void DataSyncBarrier(void)
{
	asm volatile ("mcr p15, #0, %[zero], c7, c10, #4" : : [zero] "r" (0) );
}

void FlushCache(void)
{
	asm volatile ("mcr p15, #0, %[zero], c7, c14, #0" : : [zero] "r" (0) );
}

void Pallocator_Initialize(void)
{
	// Zero out the bitmap to start with
	unsigned int i;
	for (i = 0; i < MAX_ALLOCATED_SLICES / 8; i++)
		gBitmap[i] = 0;
}

static void mark_slices(unsigned int start, unsigned int count, bool used)
{
	printf("Marking slices between %d and %d as %d\n\r", start, start + count, used);

	// Find bit to set it
	unsigned int start_byte = start / 8;
	unsigned int start_remainder = start % 8;

	unsigned int last_byte = (start + count) / 8;
	unsigned int last_remainder = (start + count) % 8;

	// Now to do some checking for cases
	if (start_byte == last_byte)
	{
		// Start and end is in the same byte
		// Example: Start: 0, count = 7, end = 7
		//	| 1 1 1 1 1 1 1 0 | 0 0 0 0 0 0	0 0 |
		//	| _____Byte0_____ | _____Byte1_____ |
		unsigned int bitsToSetAsBits = (1 << count) - 1; // Bit magic: 127

		if (used)
			gBitmap[start_byte] |= bitsToSetAsBits << (8 - start_remainder - count);
		else
			gBitmap[start_byte] &= ~(((char)bitsToSetAsBits << (8 - start_remainder - count)));
	}
	else if (last_byte == start_byte + 1 && start_remainder == 0 && last_remainder == 0)
	{
		// The block of slices fills exactly an entire byte
		gBitmap[start_byte] = used ? 0xFF : 0;
	}
	else
	{
		// Start and end is in different bytes
		unsigned int desired_start_pos = start_remainder;

		// 1. Set start
		// We know end is in a different byte, so we fill start byte, starting at desired pos
		unsigned int start_bits_to_set = 8 - desired_start_pos;
		unsigned int bitsToSetAsBits = ((1 << start_bits_to_set) - 1);
		if (used)
			gBitmap[start_byte] |= bitsToSetAsBits;
		else
			gBitmap[start_byte] &= ~bitsToSetAsBits;

		unsigned int bitsLeftToSet = count - start_bits_to_set;

		// 2. Set end (if not full)
		if (last_remainder > 0)
		{
			// We're only setting a specific amount of bits in the last byte
			unsigned int desired_end_pos = last_remainder;
			unsigned int bits_in_last_byte_to_set = last_remainder;
			bitsToSetAsBits = ((1 << bits_in_last_byte_to_set) - 1);

			if (used)
				gBitmap[last_byte] |= bitsToSetAsBits << (8 - desired_end_pos);
			else
				gBitmap[last_byte] &= ~(bitsToSetAsBits << (8 - desired_end_pos));

			bitsLeftToSet -= bits_in_last_byte_to_set;
		}

		// 3. Set all bytes inbetween
		if (bitsLeftToSet > 0)
		{
			unsigned int num_bytes_spanned = bitsLeftToSet / 8;
			unsigned int i;
			// start byte has already been set, and last byte has been set if it doesn't fill entire byte
			for (i = start_byte + 1; i <= num_bytes_spanned; i++)
				gBitmap[i] = used ? 0xFF : 0;
		}
	}
}

static int get_first_available_slice(unsigned int requestedSize)
{
	unsigned int i, j;
	int clear_bits_start = -1;
	int clear_bits_found = 0;
	
#ifdef DEBUG_MEM
	printf("Searching for first block of %d available slices.\r\n", requestedSize);
#endif

	for (i = 0; i < MAX_ALLOCATED_SLICES / sizeof(char); i++)
	{
		if ((gBitmap[i] & 0xFF) == 0xFF) // No free in this byte, skip
		{
			clear_bits_start = -1;
			clear_bits_found = 0;
		}
		else if (gBitmap[i] == 0)
		{
			if (clear_bits_start == -1)
			{
				clear_bits_start = i * (sizeof(char)* 8); // Found a new start
				clear_bits_found = sizeof(char)* 8;
			}
			else
			{
				clear_bits_found += sizeof(char)* 8; // Add to the pile
			}
		}
		else
		{
			// OK - well that sucked, just loop and see how many clear bits we can find
			for (j = sizeof(char)* 8 - 1; j > 0; j--)
			{
				if ((gBitmap[i] & (1 << j)) == 0)
				{
					// j contains the shifted offset from the right, set start to be the 0 based index from the left
					if (clear_bits_start == -1)
						clear_bits_start = ((i * (sizeof(char)* 8)) + (8 - j)) - 1;

					clear_bits_found += 1;

					if (clear_bits_found == requestedSize)
						break; // Found a free block
				}
				else
				{
					clear_bits_start = -1;
					clear_bits_found = 0;
				}
			}
		}

		// Did we find a free block?
		if (clear_bits_start != -1 && (unsigned int)clear_bits_found >= requestedSize)
			break;
	}

	assert2(clear_bits_start < 0, "Invalid slice start");

	return clear_bits_start;
}

void* palloc(unsigned int size)
{
	bool extended_size = false;
	int start_slice = -1;

	// Calculate number of slices necessary to store data
	unsigned int slice_count = size / MAX_BYTES_PER_SIZE_BYTE;
	if (size % MAX_BYTES_PER_SIZE_BYTE)
		slice_count++; // Needs to be rounded up so we can fit all the bytes

#ifdef DEBUG_MEM
	printf("palloc(%d)", size);
#endif

	// Do we need an extended size byte?
	if (slice_count < 0 || slice_count > 196) // Invalid allocation size
		return 0; // TODO: Support larger allocations, currently limited to 14^2
	else if (slice_count > 126) // We can really support 127, but we don't know if we need a size slice
		extended_size = true;

	// Can the size byte fit into the data slices?
	if (slice_count * MAX_BYTES_PER_SIZE_BYTE - (extended_size ? 2 : 1) < size)
	{
		// No dice, we need another slice so we can store size
		slice_count++; // TODO: For now, one extra slice is enough, once n size bytes are supported, this has to change
	}

	// Now find slice_count clear consecutive bits in the bitmap
	start_slice = get_first_available_slice(slice_count);

#ifdef DEBUG_MEM
	printf("Start slice: %d\r\n", start_slice);
#endif

	// Did we manage to locate a free block of slices?
	if (start_slice == -1)
		return 0; // BOO! Could not find a slice large enough

	// Calculate the address to the memory, and offset the size bytes
	unsigned char* ptr = gMemory + (start_slice * BYTES_PER_SLICE);

	// Write size byte(s)
	if (extended_size)
	{
		// first write extended size byte
		*(ptr + 1) = (1 << 7) + (slice_count & 0x7F); // Only write lower 7 bits
		*ptr = ((slice_count & 0x3F80) >> 8); // Write 7 high bits
		ptr += 2;
	}
	else
		*ptr++ = (slice_count & 0x7F);

	//Mark the slices as used in the bitmap
	mark_slices(start_slice, slice_count, true);

	return ptr;
}

void phree(void* pointer)
{
	// Free da pointah
	unsigned char* ptr = (unsigned char*)pointer;
	int num_slices = -1;

#ifdef DEBUG_MEM
	printf("phree(%d)", &pointer);
#endif

	// TODO: Allow for n number of size bytes?
	if ((*(ptr - 1) & EXTENDED_SIZE_BYTE_FLAG) == EXTENDED_SIZE_BYTE_FLAG)
	{
		unsigned int low_size = (*(ptr - 1) & 0x7F);
		unsigned int high_size = (*(ptr - 2) & 0x7F);

		num_slices = (high_size << 8) + low_size;
		ptr -= 2; // Point ptr to the start addr
	}
	else
	{
		num_slices = (*(ptr - 1) & 0x7F); // Size is stored in just one byte
		ptr--; // Point ptr to the start addr
	}

	assert(num_slices < 0 || num_slices > MAX_ALLOCATED_SLICES);

	// ptr now points to the address, figure out the offset to find the slice start number
	unsigned int start_slice = (ptr - gMemory) / BYTES_PER_SLICE;

	assert(start_slice < 0 || start_slice > MAX_ALLOCATED_SLICES - 2);

	mark_slices(start_slice, num_slices, false);
}
