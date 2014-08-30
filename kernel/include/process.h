#ifndef PROCESS_H
#define PROCESS_H

#include "paging.h"
#include "stdint.h"
#include "stdbool.h"

typedef struct thread;

#define PROCESS_START_PAGE_COUNT 8

// int main() of processes
typedef int(*process_entry_func)(void);

typedef enum {
	TS_Running, // Currently executing
	TS_Waiting, // Waiting for resource/IO etc
	TS_Ready    // Ready to be executed
} processState;

typedef enum {
	processPriorityLow,
	processPriorityMedium,
	processPriorityHigh,
	processPriorityVeryHigh
} processPriority;

typedef struct Process {
	unsigned int    id;           
	unsigned int    active;        // Whether this task is currently executing
	processState    state;         // Current state of task
    char*           name;          // Print friendly name
    char*           path;          // Path to the executable (if any)
    unsigned int*   ttb0;          // Address to page table for this task
    unsigned int*   mem_pages;     // Array of all allocated pages for the user memory
    unsigned int    num_mem_pages; // Numer of elements in mem_pages
    unsigned int*   ttb0_physical; // Physical address to the ttb0 
    unsigned int    num_ttb0_pages;// Number of pages the ttb0 spans
    ttbc_ttbr0_size ttb0_size;     // The size of the ttb0 to set TTBC split to
    uint32_t        nThreads;      // The number of threads
    struct thread** threads;       // An array of all threads in the process
    struct thread*  mainThread;    // Gets the main thread of the application
} Process;

Process* Process_Create(unsigned int entryFuncAddr, char* name, bool start);
Process* Process_CreateFromFile(char* filename, char* name, bool start);

// Frees all memory associated with the given task
void Process_Delete(Process* p);

#endif