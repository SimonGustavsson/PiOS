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

char* elf_get_sh_type(elf_shtype type)
{
    switch (type)
    {
    case SHT_NULL:
        return "Empty";
    case SHT_PROGBITS:
        return "Code";
    case SHT_SYMTAB:
        return "Symbol table";
    case SHT_STRTAB:
        return "String table";
    case SHT_RELA:
        return "Relocation (addend)";
    case SHT_REL:
        return "Relocation";
    case SHT_HASH:
        return "Hash table";
    case SHT_DYNAMIC:
        return "Dynamic";
    case SHT_NOTE:
        return "Note";
    case SHT_NOBITS:
        return "bss";
    case SHT_SHLIB:
        return "reserved (shtlib)";
    case SHT_DYNSYM:
        return "Symbol table (minimal)";
    default:
        return "reserved/unknown";
    }
}

int elf_load(unsigned char* file, int file_size, unsigned int mem_base)
{
    // Make sure the data is large enough
    if (file_size < sizeof(elf32_header) + sizeof(elf_ph))
    {
        printf("File not large enough to be an elf.\n");
        return -1;
    }

    // Parse & verify elf header
    elf32_header* header = (elf32_header*)file;    
    if (elf_verify_header_ident(header) != 0)
    {
        printf("Invalid ELF header\n");
        return -1;
    }

    unsigned int phsize = header->phentsize * header->phnum;
    elf_ph* prog_hdrs = (elf_ph*)&file[header->phoff];

    // Copy sections into memory
    unsigned int i;
    for (i = 0; i < header->phnum; i++)
    {
        elf_ph* cur = &prog_hdrs[i];
        unsigned int dest_addr = mem_base + cur->vaddr;

        if (cur->type != SHT_PROGBITS)
            continue;
        
        if (cur->filesz > 0)
        {
            // Copy data from file
            
            my_memcpy((char*)dest_addr, file[cur->offset], cur->filesz);
            printf("Section '%s' => %d bytes from 0x%h in file to memory: 0x%h\n",
                elf_get_sh_type(cur->type), 
                cur->filesz, 
                cur->offset, 
                dest_addr);
            printf("memsz: %d\n", cur->memsz);
        }
        else
        {
            // Just fill target with 0s
            my_memset((char*)dest_addr, 0, cur->memsz);
            printf("Section %s => Zeroed out %d bytes at 0x%h\n",
                elf_get_sh_type(cur->type),
                cur->memsz, 
                dest_addr);
        }
    }

    return 0;
}