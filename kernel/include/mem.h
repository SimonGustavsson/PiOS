// Cover the entire 32-bit range
#define PAGE_SIZE 4096
#define MAX_AVAILABLE_MEMORY 536870912 // 512MB
#define MAX_ALLOCATED_PAGES (MAX_AVAILABLE_MEMORY / PAGE_SIZE)

//#define PAGEMEM_DEBUG

int mem_init(void);

// Returns the next available page
int mem_nextFree(void);

// Returns the address of the first page in the first series of available num_pages
int mem_nextFreeContiguous(unsigned int num_pages);

// Frees the page that covers the given physical address
void mem_free(unsigned int addr);

int mem_reserve(unsigned int startAddr, unsigned int size);
// Marks the page that covers addr as used
int mem_reserveSingle(unsigned int addr);
int mem_reserveRange(unsigned int startAddr, unsigned int endAddr);

unsigned int mem_getAvailable(void);
unsigned int mem_getAllocated(void);
void mem_printUsage(void);