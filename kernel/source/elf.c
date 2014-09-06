#include "elf.h"
#include "memory.h"
#include "types/string.h"
#include "util/utilities.h"
#include "debugging.h"
#include "stddef.h"
#include "util/memutil.h"

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

int elf_get_func_info(char* elf, int elf_size, func_info** info)
{
    elf32_header* header = (elf32_header*)elf;

    if (elf_verify_header_ident(header) != 0)
        return -1;

    elf_shdr* shdrs = (elf_shdr*)&elf[header->shoff];

    // Find the string table so we can resolve function names
    char* strtab = NULL;
    int numSymbols = 0;
    elf_sym* symtab = NULL;
    unsigned int i;
    for (i = 0; i < header->shnum; i++)
    {
        // TODO: This doesn't deal well with multiple STRTAB/SYMTAB
        if (shdrs[i].type == SHT_STRTAB && i != header->shstrndx)
        {
            strtab = elf + shdrs[i].offset;
        }

        if (shdrs[i].type == SHT_SYMTAB)
        {
            symtab = (elf_sym*)(elf + shdrs[i].offset);
            numSymbols = shdrs[i].size / shdrs[i].entsize;
        }
    }

    // Count number of functions
    int numFunctions = 0;
    for (i = 0; i < numSymbols; i++)
    {
        if (ELF32_ST_TYPE(symtab[i].info) == STT_FUNC && symtab[i].name > 0)
            numFunctions++;
    }

    // We now know the number of functions, we have the symbol table and the string table
    // Time to combine it all into the result
    *info = (func_info*)palloc(numFunctions * sizeof(func_info));
    int curFuncIndex = 0;
    for (i = 0; i < numSymbols; i++)
    {
        // Skip irrelevant symbols
        if (ELF32_ST_TYPE(symtab[i].info) != STT_FUNC || symtab[i].name == 0)
            continue;

        elf_sym* sym = &symtab[i];

        func_info* curFun = &(*info)[curFuncIndex++];
        curFun->address = sym->value;

        char* name = strtab + sym->name;
        int nameLen = my_strlen(name);

        curFun->name = (char*)palloc(nameLen + 1);
        my_strcpy_s(curFun->name, nameLen + 1, name);
    }

    return numFunctions;
}

int elf_load(char* file, int file_size, unsigned int mem_base)
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

    elf_shdr* shdrs = (elf_shdr*)&file[header->shoff];

    // Copy sections into memory
    unsigned int i;
    for (i = 0; i < header->shnum; i++)
    {
        elf_shdr* cur = &shdrs[i];

        if ((cur->flags & SHF_ALLOC) != SHF_ALLOC)
            continue; // Skip sections that don't get loaded into memory

        unsigned int dest_addr = mem_base + cur->addr;

        if (cur->type == SHT_NOBITS)
            my_memset((char*)dest_addr, 0, cur->size);
        else
            my_memcpy((char*)dest_addr, &file[cur->offset], cur->size);
    }

    return 0;
}
