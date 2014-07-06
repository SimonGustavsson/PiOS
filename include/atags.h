typedef enum {

    // Empty tag used to end list
    ATAG_NONE       = 0x00000000,

    // Start tag used to begin list
    ATAG_CORE       = 0x54410001,

    // Tag used to describe a physical area of memory.
    ATAG_MEM        = 0x54410002,

    // Tag used to describe VGA text type displays
    ATAG_VIDEOTEXT  = 0x54410003,

    // Tag describing how the ramdisk will be used by the kernel
    ATAG_RAMDISK    = 0x54410004,

    // Tag describing the physical location of the compressed ramdisk image
    ATAG_INITRD2    = 0x54420005,

    // Tag with 64 bit serial number of the board
    ATAG_SERIAL     = 0x54410006,

    // Tag for the board revision
    ATAG_REVISION   = 0x54410007,

    // Tag describing parameters for a framebuffer type display
    ATAG_VIDEOLFB   = 0x54410008,

    // Tag used to pass the commandline to the kernel
    ATAG_CMDLINE    = 0x54410009
} atag_type;

typedef struct {

    // bit 0 = read - only
    unsigned int flags;
    // Systems page size (usually 4k)
    unsigned int pagesize;

    // Root device number
    unsigned int rootdev;
} atag_core;

typedef struct {

    // Size of memory area
    unsigned int     size;

    // Physical start address 
    unsigned int     start;
} atag_mem;

typedef struct {

    // Display width
    unsigned char              x;

    // Display height
    unsigned char              y;
    unsigned short             video_page;
    unsigned char              video_mode;
    unsigned char              video_cols;
    unsigned short             video_ega_bx;
    unsigned char              video_lines;
    unsigned char              video_isvga;
    unsigned short             video_points;
} atag_videotext;

typedef struct {

    // bit 0 = load, bit 1 = prompt
    unsigned int flags;

    // Decompressed ramdisk size in _kilo_ bytes 
    unsigned int size;

    // Starting block of floppy-based RAM disk image 
    unsigned int start;
} atag_ramdisk;

typedef struct {

    // Physical start address 
    unsigned int start;

    // Size of compressed ramdisk image in bytes
    unsigned int size;
} atag_initrd2;

typedef struct {
    unsigned int low;
    unsigned int high;
} atag_serialnr;

typedef struct {

    // Board revision
    unsigned int rev;
} atag_revision;

typedef struct {
    // 1 is the minimum size
    char    cmdline[1];
} atag_cmdline;

typedef struct {
    unsigned short             lfb_width;
    unsigned short             lfb_height;
    unsigned short             lfb_depth;
    unsigned short             lfb_linelength;
    unsigned int               lfb_base;
    unsigned int               lfb_size;
    unsigned char              red_size;
    unsigned char              red_pos;
    unsigned char              green_size;
    unsigned char              green_pos;
    unsigned char              blue_size;
    unsigned char              blue_pos;
    unsigned char              rsvd_size;
    unsigned char              rsvd_pos;
} atag_videolfb;

typedef struct {
    // Length of tag in words including this header
    unsigned int size;

    // Tag type (see atag_type)
    unsigned int tag;
} atag_header;

typedef struct {
    atag_header hdr;

    union {
       atag_core         core;
       atag_mem          mem;
       atag_videotext    videotext;
       atag_ramdisk      ramdisk;
       atag_initrd2      initrd2;
       atag_serialnr     serialnr;
       atag_revision     revision;
       atag_videolfb     videolfb;
       atag_cmdline      cmdline;
    } u;
} atag;

void atags_parse(int* addr);