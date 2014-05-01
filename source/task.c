#include "stddef.h"
#include "task.h"
#include "hardware/mmu_c.h"
#include "elf.h"
#include "fs/fs.h"
#include "memory.h"
#include "types/string.h"
#include "taskScheduler.h"

// This gets called when a task is started and is what actually branches
// into the tasks main function
void Task_StartupFunction(Task* task)
{
	// Branch into the tasks main function
    task->result = task->entry();

	// Do system call to tell the scheduler this task is no longer running
}

int Task_GetElfData(char* filename, char** buffer)
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

task_entry_func Task_LoadElf(char* filename, unsigned int addr)
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

Task* Task_Create(task_entry_func entry, char* name)
{
    Task* t = (Task*)palloc(sizeof(Task));

    t->active = 0;
    t->entry = entry;
    t->priority = TaskPriorityMedium;
    t->state = Ready;
    t->timeElapsed = 0;
    t->started = 0;
    t->id = TaskScheduler_GetNextTID();
    t->name = name;

	return t;
}