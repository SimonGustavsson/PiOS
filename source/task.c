#include "stddef.h"
#include "task.h"
#include "hardware/mmu_c.h"
#include "elf.h"
#include "fs/fs.h"
#include "memory.h"
#include "types/string.h"
#include "taskScheduler.h"
#include "mem.h"
#include "hardware/paging.h"
#include "util/memutil.h"

static int Task_GetElfData(char* filename, char** buffer)
{
    int handle = fs_open(filename, file_read);
    if (handle == INVALID_HANDLE)
    {
        printf("Failed to open %s\n", filename);
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

static task_entry_func Task_LoadElf(char* filename, unsigned int addr)
{
    char* file_data = NULL;
    unsigned int file_size = Task_GetElfData(filename, &file_data);

    if (file_size == -1)
    {
        printf("Failed to load '%s'\n", filename);
        return NULL;
    }

    elf32_header* hdr = (elf32_header*)file_data;
    if (elf_verify_header_ident(hdr) != 0)
    {
        return NULL;
    }

    if (elf_load(file_data, file_size, addr) != 0)
    {
        return NULL;
    }
    
    return (task_entry_func)addr;
}

Task* Task_Create(char* filename, char* name)
{
    Task* t = (Task*)palloc(sizeof(Task));

    t->active = 0;
    t->priority = TaskPriorityMedium;
    t->state = Ready;
    t->timeElapsed = 0;
    t->started = 0;
    t->id = TaskScheduler_GetNextTID();
    t->path = NULL;
    if (Task_InitializeMemory(t) != 0)
    {
        phree(t);
        return NULL;
    }

    // NOTE: This assumes the kernel ttb1 identity maps physical memory to KERNEL_VA_START
    unsigned int task_memory_start = (KERNEL_VA_START + t->mem_pages[0]);
    
    task_entry_func func = Task_LoadElf(filename, task_memory_start);
    if (func == NULL)
    {
        printf("Scheduler_Enqueue: Failed to load elf '%s'\n", filename);

        Task_Delete(t);

        phree(t);

        return NULL;
    }

    t->name = name;
    t->path = (char*)palloc(my_strlen(filename));
    my_strcpy(filename, t->path);

    t->entry = func;

    return t;
}

int Task_InitializeMemory(Task* t)
{
    unsigned int* pages = (unsigned int*)palloc(8 * sizeof(unsigned int*));
    unsigned int pages_allocated = 0;

    // Create TTB0 - 128byte TT - Covering 32MB of Virtual Memory - this fits neatly into 8 pages
    int physical_tt = mem_nextFreeContiguous(TASK_INITIAL_PAGE_COUNT);

    if (physical_tt == -1)
        goto cleanup;

    // Zero out all the pages - (Translation Fault for everything)
    unsigned int i;
    for (i = 0; i < TASK_INITIAL_PAGE_COUNT; i++)
    {
        unsigned int* tt_page = (unsigned int*)(KERNEL_VA_START + physical_tt + (PAGE_SIZE * i));

        unsigned int j;
        for (j = 0; j < PAGE_SIZE; j++)
            tt_page[j] = 0;
    }

    t->num_ttb0_pages = TASK_INITIAL_PAGE_COUNT;

    // Temporarily map it into kernel space so we can mess with it
    //map_page(kernel_ttb1, TTB_SIZE_4GB_SIZE, physical_tt, KERNEL_VA_START + physical_tt, PAGE_BUFFERABLEERABLE | PAGE_CACHEABLEABLE | PT_TYPE_SMALLPAGE | SMALLPAGE_AP_RW);

    unsigned int* temp_va_tt = (unsigned int*)(KERNEL_VA_START + physical_tt);

    // Invalidate TLB so that we write to the correct location
    FlushTLB((unsigned int)temp_va_tt);

    // We can now write to the table
    for (i = 0; i < TASK_INITIAL_PAGE_COUNT; i++)
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

    unsigned int lvl1_entries_num_pages = TTB_SIZE_32MB_SIZE / PAGE_SIZE;

    // Might not fit exactly into pages, add one if the level 1 entries spills over into a page
    lvl1_entries_num_pages += (TTB_SIZE_32MB_SIZE % PAGE_SIZE) == 0 ? 0 : 1;

    unsigned int* level2_entries_start = (unsigned int*)(((unsigned int)temp_va_tt) + (lvl1_entries_num_pages * PAGE_SIZE));

    t->ttb0_size = ttbc_128bytes;
    t->ttb0 = temp_va_tt;
    t->ttb0_physical = (unsigned int)physical_tt;
    t->mem_pages = pages;
    t->num_mem_pages = TASK_INITIAL_PAGE_COUNT;

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
    for (i = 0; i < TASK_INITIAL_PAGE_COUNT; i++)
        mem_free(physical_tt + (i * 0x1000));

    phree(pages);
    phree(t);

    return -1;
}

void Task_Delete(Task* task)
{
    // Return the user memory  pages used
    unsigned int i;
    for (i = 0; i < task->num_mem_pages; i++)
        mem_free(task->mem_pages[i]);

    for (i = 0; i < task->num_ttb0_pages; i++)
        mem_free(task->ttb0_physical[i]);

    if (task->path != NULL)
        phree(task->path);

    phree(task->mem_pages);
}
