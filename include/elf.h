#define EI_NIDENT 16

/*
    Elf data sizes and their meanings:
    ___Name________|_Size___|_Alignment_|_______Purpose___________
    |Elf32_Addr    |   4    |    4      | Unsigned Program Address|
    |Elf32_Half    |   2    |    2      | Unsigned medium integer |
    |Elf32_Off     |   4    |    4      | Unsigned file offset    |
    |Elf32_Sword   |   4    |    4      | Signed large integer    |
    |Elf32_Word    |   4    |    4      | Unsigned large integer  |
    |unsigned char |   1    |    1      | Unsigned small integer  |

    Sizes used in this code:
    Elf32_Addr   = unsigned int
    Elf32_Half   = unsigned short
    Elf32_Off    = unsigned int
    Elf32_Sword  = int
    Elf32_Word   = unsigned int

*/

typedef enum {
    ET_NONE = 0,
    ET_REL = 1,
    ET_EXEC = 2,
    ET_DYN = 3,
    ET_CORE = 4,
    ET_LOPROC = 0xFF00,
    ET_HIPROC = 0xFFFF
} elf_type;

typedef enum {
    EM_NONE = 0,  // No Machine
    EM_M32 = 1,   // AT&T WE 32100
    EM_SPARC = 2, // SPARC
    EM_386 = 3,   // Intel 80386
    EM_68K = 4,   // Motorola 68000
    EM_88K = 5,   // Motorola 88000
    // Note: No 6!
    EM_860 = 7,   // Intel 80860
    EM_MIPS =  8  // MIPS RS3000
} elf_machine;

typedef enum {
    EV_NONE = 0,    // Invalid Version
    EV_CURRENT = 1, // Current Version
} elf_version;

typedef enum {
    ELFCLASS_NONE = 0, // Invalid class
    ELFCLASS_32 = 1,   // 32-bit objects
    ELFCLASS_64 = 2    // 64-bit objects
} elf_class;

typedef enum {
    ELFDATA_NONE = 0, // Invalid data
    ELFDATA_LSB = 1,  // Data is least-significant byte first. ie: 0x0102 is [02] [01]
    ELFDATA_MSB = 2,  // Data is most-significant byte first. ie: 0x0102 is [01] [02]
} elf_data;

/* 
    elf_header ident field explanation:
    ident[0] = File identification    - Should be 0x7F
    ident[1] = File identification    - Should be 'E'
    ident[2] = File identification    - Should be 'L'
    ident[3] = File identification    - Should be 'F'
    ident[4] = File class             - see elf_class
    ident[5] = Data encoding          - see elf_data
    ident[6] = File version           - see elf_version
    ident[7] = Start of padding bytes - Marks beginning of first unused byte in the ident array
    ident[16] = Size of ident field (this array)
*/
typedef struct { 
    unsigned char  ident[EI_NIDENT]; 
    unsigned short type;              // See elf_type
    unsigned short machine; 
    unsigned short version; 
    int            entry;             // Address of entry point to start execution
    int            phoff;             // Program header table file offset (in bytes)
    int            shoff;             // Section header table file offset (in bytes)
    int            flags;             // See elf_flags (note: flags are architecture specific)
    unsigned short ehsize;            // Header size (in bytes)
    unsigned short phentsize;         // Program header table entry size (in bytes)
    unsigned short phnum;             // Program header table entry count
    unsigned short shentsize;         // Section header table entry size (in bytes)
    unsigned short shnum;             // Section header table entry count
    unsigned short shstrndx;          // Section header table index of entry associated with the section name string table
} elf32_header;

typedef struct {

} elf_shentry;

// These indices in the section header table are reserved
// The file will have no sections for these indices
typedef enum {
    SHN_UNDEF     = 0,        // Undefined - Nothing to see here...
    SHN_LORESERVE = 0xFF00,   // Lower bound of reserved indices
    SHN_LOPROC    = 0xFF00,   // Values from this to HIPROC are reserved for
    SHN_HIPROC    = 0xFF1F,   //    Processor specific semantics
    SHN_ABS       = 0xFFF1,   // Specifies absolute values for for the corresponding references, symbols defiend relative to SHN_ABS are absolute (not affected by relocation)
    SHN_COMMON    = 0xFFF2,   // Common symbols such as C unallocated variables
    SHN_HIRESERVE = 0xFFFF    // Upper bound of reserved indices
} elf_shspecial;

typedef struct {
    unsigned int name;      // Name of section - This is an index into the header string table
    unsigned int type;      // see elf_shtype
    unsigned int flags;     // see elf_shflag
    unsigned int addr;      // If section will appear as a process in memory this field contains the address the first byte of the section should be loaded to
    unsigned int offset;    // File offset of this section. Note: Does not apply to section SHT_NOBITS
    unsigned int size;      // Section size (in bytes). Note: SHT_NOBITS might have a value, but does not occupy any space
    unsigned int link;      // Contains the section header table index link. See below on how to interpret this field
    unsigned int info;      // See below on how to interpret this field
    unsigned int addralign; // Specifies alignment requirement. Valid values: 0 and ^2. Value 0 and 1 means no alignment constraint
    unsigned int entsize;   // If section holds fixed-size entries such as a symbol table, this field gives the size of each entry (in bytes)
} elf_sh;

typedef enum {
    SHT_NULL     = 0,           // Inactive, no associated section
    SHT_PROGBITS = 1,           // Information defined by program
    SHT_SYMTAB   = 2,           // Section holds symbol table, one symbol table per file
    SHT_STRTAB   = 3,           // Section holds string table, file may have multiple
    SHT_RELA     = 4,           // Section holds relocation information see elf_rela
    SHT_HASH     = 5,           // Section holds a hash table, one hash table per file
    SHT_DYNAMIC  = 6,           // Section holds information for dynamic linking
    SHT_NOTE     = 7,           // Section holds information that marks the file in some way
    SHT_NOBITS   = 8,           // Occupies no space but acts like SHT_PROGBITS
    SHT_REL      = 9,           // Section holds relocation entries without explicit addends, file may have multiple
    SHT_SHLIB    = 10,          // Reserved but unspecified semantics
    SHT_DYNSYM   = 11,          // Same as SHT_SYMTAB but contains a minimal set of dynamic linking symbols
    SHT_LOPROC   = 0x70000000,  // Values from this to SHT_HIPROC
    SHT_HIPROC   = 0x7FFFFFFF,  //    are reserved for processor-specific semantics
    SHT_LOUSER   = 0x80000000,  // Lower bound of the range of indexes reserved for application programs
    SHT_HIUSER   = 0xFFFFFFFF   // Upper bound of the range of indexes reserved for application programs
} elf_shtype;

typedef enum {
    SHF_WRITE     = 0x1,        // Section contains data that should be writable during execution
    SHF_ALLOC     = 0x2,        // Section occupies memory during process execution
    SHF_EXECINSTR = 0x4,        // Section contains executable instructions
    SHF_MASKPROC  = 0xF0000000  // All bits reserved for processor-specific semantics
} elf_shflags;

/*                     sh_link / sh_info interpretation:
    _____________________________________________________________________________
    |__sh_type______|_________sh_link______________|_sh_info_____________________|
    |SHT_DYNAMIC    | The section header index of  |                             |
    |               | The strings table used by    |         0                   |
    |_______________|_entries in the section_______|_____________________________|
    |   SHT_HASH    | The section header index of  |                             |
    |               | the string table used by     |         0                   |
    |_______________|_entries in the section.______|_____________________________|
    |               | The section header index of  | The section header index of |
    |  SHT_REL      | the associated symbol table. | the section to which the    |
    |__SHT_RELA_____|______________________________|_reallocation applies________|
    | SHT_SYMTAB /  | The section header index of  | One greater than the symbol |
    | SHT_DYNTAB    | the associated string table  | table index of the last     |
    |_______________|______________________________|_local symbol (binding STB_L_|
    |_____Other_____|__________SHN_UNDEF___________|___________0_________________|
*/

/*  Special sections
_______________________________________________________________________________________________
|____Name_____|____Type_______|______Attributes____________|_____________Details_______________
|   .bss      | SHT_NOBITS    | SHF_ALLOC + SHF_WRITE      | Uninitialized program memory
|   .comment  | SHT_PROGBITS  | none                       | Version control information
|   .data     | SHT_PROGBITS  | SHF_ALLOC + SHF_WRITE      | Initialized program memory
|   .data1    | SHT_PROGBITS  | SHF_ALLOC + SHF_WRITE      | Initialized program memory
|   .debug    | SHT_PROGBITS  | none                       | Symbols for debugging
|   .dynamic  | SHT_DYNAMIC   | see below                  | Dynamic linking information
|   .dynstr   | SHT_STRTAB    | SHF_ALLOC                  | Strings needed for dynamic linking
|   .dynsym   | SHT_DYNSYM    | SHF_ALLOC                  | Dynamic linking symbol table
|   .fini     | SHT_PROGBITS  | SHF_ALLOC + SHF_EXECINSTR  | Process destructor code
|   .got      | SHT_PROGBITS  | see below                  | Global offset table
|   .hash     | SHT_HASH      | SHF_ALLOC                  | Symbol hash table
|   .init     | SHT_PROGBITS  | SHF_ALLOC + SHF_EXECINSTR  | Process constructor code
|   .interp   | SHT_PROGBITS  | see below                  | Path name of program interpreter
|   .line     | SHT_PROGBITS  | none                       | Line nr info for symbolic debugging
|   .note     | SHT_NOTE      | none                       | Holds note information
|   .plt      | SHT_PROGBITS  | see below                  | Procedure linking table
|   .relname  | SHT_REL       | see below                  | Relocation information
|   .relaname | SHT_RELA      | see below                  | Relocation information
|   .rodata   | SHT_PROGBITS  | SHF_ALLOC                  | Read-only program data
|   .rodata1  | SHT_PROGBITS  | SHF_ALLOC                  | Read-only program data
|   .shstrtab | SHT_STRTAB    | none                       | Section name table
|   .strtab   | SHT_STRTAB    | see below                  | Holds name strings, most commonly name associated with symbol table entries
|   .symtab   | SHT_SYMTAB    | see below                  | Symbol table
|___.text_____|_SHT_PROGBITS__|_SHF_ALLOC + SHF_EXECINSTR__| Executable instructions of the program
*/

/*             String table
    First index into string table is a \0 character
    String index of 0 means no name or a null entry
    Non-zero indexes are NOT valid

    Example table:
    __________________________________________________________
    |__Index__+0___+1___+2___+3___+4___+5___+6___+7___+8___+9_|
    |    0  | \0 | n  | a  | m  | e  | .  | \0 | V  | a  | r  |  
    |   10  | i  | a  | b  | l  | e  | \0 | a  | b  | l  | e  |
    |___20__|_\0_|_\0_|_x__|_x__|_\0_|____|____|____|____|____|
    ____________________________
    |___Index___|___String______|
    |    0      |    <empty>    |
    |    1      |    'name.'    |
    |    7      |  'Variable'   |
    |   11      |    'able'     |
    |   16      |    'able'     |
    |___24______|____<NULL>_____|

*/

/*            Symbol Table

*/

// Symbol table entry
typedef struct {
    unsigned int   name;   // Index into the string table if non-zero, otherwise it has no name
    unsigned int   value;  // Value of the associated symbol depending on context, absolute value, address etc
    unsigned int   size;   // Size of the symbol
    unsigned char  info;   // The symbols type and binding attributes
    unsigned char  other;  // 0, value has no meaning
    unsigned short shndx;  // Index of the section header this symbol is relevant to
} elf_sym;

typedef enum {
    STB_LOCAL  = 0, // Local symbols are not visible outside the object file containing their definition
    STB_GLOBAL = 1, // Global symbols are visible to all objects being combined
    STB_WEAK   = 2, // Weak symbols resembles global symbols, but thei definitions have lower precedence
    STB_LOPROC = 3, // Values in range STB_LOPROC to
    STB_HIPROC = 4  //     STB_HIPROC are reserved for processor specific semantics
} elf_stb;

#define ELF32_ST_BIND(i) ((i)>>4) 
#define ELF32_ST_TYPE(i) ((i)& 0x  f)
#define ELF32_ST_INFO(b, t) (((b) << 4) + ((t) & 0xF)

typedef enum {
    STT_NOTYPE   = 0,  // Symbol type not specified
    STT_OBJECT   = 1,  // Data object, variable etc
    STT_FUNC     = 2,  // Function or other executable code
    STT_SECTION  = 3,  // Symbol associated with section, typically for relocation
    STT_FILE     = 4,  // Conventionally this contains the name of the source file
    STT_LOPROC   = 13, // Values between STT_LOPROC and
    STT_HIPROC   = 15  //    STT_HIPROC are reserved for processor specific semantics
} elf_stt;

// More info on symbol tables in 3-19

typedef struct {
    // Location at which to apply the relocation action. For a relocatable file, 
    // the value is the byte offset from the beginning of the section to the storage unit affected by the relocation.
    unsigned int offset; 

    // This member gives both the symbol table index with respect to which the relocation must be made, and the type of relocation to apply. 
    unsigned int info;
} elf_rel;

typedef struct {
    // Location at which to apply the relocation action. For a relocatable file, 
    // the value is the byte offset from the beginning of the section to the storage unit affected by the relocation.
    unsigned int offset;

    // This member gives both the symbol table index with respect to which the relocation must be made, and the type of relocation to apply. 
    unsigned int info;

    // Specifies a constant addend used to compute the value to be stored into the relocatable field
    unsigned short addend;
} elf_rela;
