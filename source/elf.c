#include "elf.h"
#include "memory.h"
#include "types/string.h"
#include "util/utilities.h"

int elf_verify_header_ident(elf32_header* header)
{
    // Header size
    if (header->ehsize != sizeof(elf32_header))
    {
        printf("Unexpected header size\n");
        return -1;
    }

    // Magic bytes
    if (*(int*)header->ident != ELF_MAGIC)
    {
        printf("Unexpected elf header\n");
        return -1;
    }

    // Version
    if (header->ident[6] != EV_CURRENT)
    {
        printf("Invalid elf version\n");
        return -1;
    }

    // Max segments
    if (header->phnum > EXE_MAX_SEGMENTS)
    {
        printf("Too many segments in header\n");
        return -1;
    }

    // Machine type
    if (header->machine != EM_ARM)
    {
        printf("Invalid machine type\n");
        return -1;
    }

    return 0;
}

int elf_get_strtab(unsigned char* table_data, unsigned int size, char*** result)
{
    int strings_to_find = 0;
    unsigned int i;
    for (i = 0; i < size; i++)
    {
        if (*(table_data + i) == 0)
            strings_to_find++;
    }

    unsigned char** res = (unsigned char**)palloc(sizeof(char*)* strings_to_find);
    unsigned int strings_found = 0;
    unsigned char* str_start = table_data;
    for (i = 0; i < size; i++)
    {
        if (*(table_data + i) == 0)
        {
            res[strings_found] = str_start;
            str_start = table_data + i + 1;
            strings_found++;
        }
    }

    *result = (char**)res;

    return strings_found;
}

int elf_load(unsigned char* file, int file_size, unsigned int addr)
{
    // Make sure the data is large enough
    if (file_size < sizeof(elf32_header) + sizeof(elf_ph))
    {
        printf("File not large enough to be an elf.\n");
        return -1;
    }

    // Parse & verify elf header
    elf32_header* header = (elf32_header*)palloc(sizeof(elf32_header));
    my_memcpy(header, file, sizeof(elf32_header));
    
    if (elf_verify_header_ident(header) != 0)
    {
        printf("Invalid ELF header\n");
        return -1;
    }

    // Throw program header into struct we can interpret
    unsigned int phsize = header->phentsize * header->phnum;
    elf_ph* ph = (elf_ph*)palloc(phsize);
    my_memcpy(ph, &file[header->phoff], phsize);

    // TODO: Actually do something

    return 0;
}