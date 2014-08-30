#include "elf.h"
#include "mem.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"
#include "stddef.h"
#include "fs/fs.h"
#include "paging.h"
#include "types/string.h"
#include "util/memutil.h"

static int Process_GetElfData(char* filename, char** buffer)
{
    int handle = fs_open(filename, file_read);
    if (handle == INVALID_HANDLE)
    {
        return -1;
    }

    // Find the file size
    fs_seek(handle, 0, seek_end);
    unsigned int fileSize = fs_tell(handle) & 0xFFFFFFFF;
    fs_seek(handle, 0, seek_begin);

    *buffer = (char*)palloc(fileSize);

    if (*buffer == 0)
    {
        printf("Failed to allocate buffer for '%s'\n", filename);
        return -1;
    }

    // Read the entire file
    fs_read(handle, *buffer, fileSize);

    // Don't need the file anymore
    fs_close(handle);

    return fileSize;
}

static process_entry_func Process_LoadElf(char* filename, unsigned int addr)
{
    char* file_data = NULL;
    unsigned int file_size = Process_GetElfData(filename, &file_data);

    if (file_size == -1)
    {
        return NULL;
    }

    func_info* functions;
    int numFuncs = elf_get_func_info(file_data, file_size, &functions);

    printf("%s has %d functions!\n", filename, numFuncs);
    unsigned int i;
    for (i = 0; i < numFuncs; i++)
    {
        phree(functions[i].name);
    }
    phree(functions);

    elf32_header* hdr = (elf32_header*)file_data;
    if (elf_verify_header_ident(hdr) != 0)
    {
        return NULL;
    }

    if (elf_load(file_data, file_size, addr) != 0)
    {
        return NULL;
    }
    
    return (process_entry_func)addr;
}

static int Process_InitializeMemory(Process* t)
{
    unsigned int* pages = (unsigned int*)palloc(8 * sizeof(unsigned int*));
    unsigned int pages_allocated = 0;

    // Create TTB0 - 128byte TT - Covering 32MB of Virtual Memory - this fits neatly into 8 pages
    int physical_tt = mem_nextFreeContiguous(PROCESS_START_PAGE_COUNT);

    //printf("Initializing PT for process at 0x%h\n", physical_tt);

    if (physical_tt == -1)
        goto cleanup;

    // Zero out all the pages - (Translation Fault for everything)
    unsigned int i;
    for (i = 0; i < PROCESS_START_PAGE_COUNT; i++)
    {
        unsigned int* tt_page = (unsigned int*)(KERNEL_VA_START + physical_tt + (PAGE_SIZE * i));

        unsigned int j;
        for (j = 0; j < PAGE_SIZE; j++)
            tt_page[j] = 0;
    }

    t->num_ttb0_pages = PROCESS_START_PAGE_COUNT;

    // Temporarily map it into kernel space so we can mess with it
    //map_page(kernel_ttb1, TTB_SIZE_4GB_SIZE, physical_tt, KERNEL_VA_START + physical_tt, PAGE_BUFFERABLEERABLE | PAGE_CACHEABLEABLE | PT_TYPE_SMALLPAGE | SMALLPAGE_AP_RW);

    unsigned int* temp_va_tt = (unsigned int*)(KERNEL_VA_START + physical_tt);

    // Invalidate TLB so that we write to the correct location
    FlushTLB((unsigned int)temp_va_tt);

    // We can now write to the table
    for (i = 0; i < PROCESS_START_PAGE_COUNT; i++)
    {
        unsigned int page = mem_nextFree();

        if (page == -1)
            goto cleanup;

        // Store page so we can free it if something goes wrong
        pages[i] = page;

        // Keep track of number of pages so we can clean up if something goes wrong
        pages_allocated++;

        map_page(temp_va_tt, TTB_SIZE_32MB_SIZE, page, USR_VA_START + (i * 0x1000), PAGE_BUFFERABLE | PAGE_CACHEABLE | PAGE_AP_K_RW_U_RW);

        FlushTLB(USR_VA_START + (i * 0x1000));
    }

    //printf("Initialized Process memory 0x%h -> 0x%h\n", pages[0], pages[pages_allocated - 1]);

    unsigned int lvl1_entries_num_pages = TTB_SIZE_32MB_SIZE / PAGE_SIZE;

    // Might not fit exactly into pages, add one if the level 1 entries spills over into a page
    lvl1_entries_num_pages += (TTB_SIZE_32MB_SIZE % PAGE_SIZE) == 0 ? 0 : 1;

    unsigned int* level2_entries_start = (unsigned int*)(((unsigned int)temp_va_tt) + (lvl1_entries_num_pages * PAGE_SIZE));

    t->ttb0_size = ttbc_128bytes;
    t->ttb0 = temp_va_tt;
    t->ttb0_physical = (unsigned int*)physical_tt;
    t->mem_pages = pages;
    t->num_mem_pages = PROCESS_START_PAGE_COUNT;

    *(unsigned int*)(t->mem_pages[0] + KERNEL_VA_START) = t->id;

    return 0;

cleanup:
    printf("Failed to allocate enough pages to create task. Out of memory?\n");

    mem_printUsage();

    // Free up user available pages we allocate
    for (i = 0; i < pages_allocated; i++)
    {
        mem_free(pages[i]);
        pages[i] = 0;
    }

    // Free the TTB0 pages
    for (i = 0; i < PROCESS_START_PAGE_COUNT; i++)
        mem_free(physical_tt + (i * 0x1000));

    phree(pages);
    phree(t);

    return -1;
}

Process* Process_Init(char* name)
{
    Process* p = (Process*)palloc(sizeof(Process));

    p->active = 0;
    p->state = TS_Ready;
    p->id = Scheduler_GetNextTID();
    p->path = NULL;
    if (Process_InitializeMemory(p) != 0)
    {
        phree(p);
        return NULL;
    }

    p->name = (char*)palloc(my_strlen(name));
    my_strcpy(name, p->name);

    return p;
}

static bool Process_CreateMainThread(Process* p, thread_entry entry)
{
    printf("Created process '%s', creating main thread, entry: 0x%h\n", p->name, entry);

    p->threads = (thread*)palloc(sizeof(thread));
    thread* mainThread = thread_createWithOwner(entry, p);
    mainThread->name = (char*)palloc(my_strlen("main"));
    my_strcpy("main", mainThread->name);

    if (mainThread == NULL)
    {
        printf("Failed to create main thread for process.\n");
        phree(p->threads);
        return false;
    }

    p->threads[0] = mainThread;
    p->mainThread = mainThread;
    p->nThreads = 1;
}

Process* Process_CreateFromFile(char* filename, char* name, bool start)
{
    Process* p = Process_Init(name);

    if (p == NULL)
    {
        printf("Failed to create task '%s'\n", name);
        return NULL;
    }

    // Load executable
    process_entry_func func = Process_LoadElf(filename, (KERNEL_VA_START + p->mem_pages[0]));
    if (func == NULL)
    {
        printf("Failed to load executable '%s'\n", filename);
        Process_Delete(p);
        phree(p);
        return NULL;
    }

    p->path = (char*)palloc(my_strlen(filename));
        
    if (!Process_CreateMainThread(p, (thread_entry)func))
    {
        printf("Failed to create main thread for process '%s'\n", name);
        Process_Delete(p);
        phree(p);
        return NULL;
    }

    if (start)
        Scheduler_Enqueue(p);

    return p;
}

Process* Process_Create(unsigned int entryFuncAddr, char* name, bool start)
{
    Process* p = Process_Init(name);

    if (p == NULL)
    {
        printf("Failed to create task '%s'\n", name);
        return NULL;
    }

    Process_CreateMainThread(p, (thread_entry)entryFuncAddr);

    if (start)
        Scheduler_Enqueue(p);

    return p;
}

void Process_Delete(Process* p)
{
    // Return the user memory  pages used
    unsigned int i;
    for (i = 0; i < p->num_mem_pages; i++)
        mem_free(p->mem_pages[i]);

    for (i = 0; i < p->num_ttb0_pages; i++)
        mem_free(p->ttb0_physical[i]);

    if (p->path != NULL)
        phree(p->path);

    phree(p->mem_pages);
}
