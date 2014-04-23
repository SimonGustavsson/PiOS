#ifndef TASK_H
#define TASK_H

#include "hardware/mmu.h"

#define MAX_TASK_MEMORY_MB 10

typedef int(*task_entry_func)(void);

typedef enum {
	Running, // Currently executing
	Waiting, // Waiting for resource/IO etc
	Ready    // Ready to be executed
} taskState;

typedef enum {
	TaskPriorityLow,
	TaskPriorityMedium,
	TaskPriorityHigh,
	TaskPriorityVeryHigh
} taskPriority;

// Stores process state when another process is being scheduled
typedef struct {
	unsigned int r0;
	unsigned int r1;
	unsigned int r2;
	unsigned int r3;
	unsigned int r4;

	unsigned int r5;
	unsigned int r6;
	unsigned int r7;
	unsigned int r8;
	unsigned int r9;

	unsigned int r10;
	unsigned int r11;
	unsigned int r12;
	unsigned int r13; // SP
	unsigned int r14; // LR
	unsigned int r15; // PC
} registers;

typedef struct {
	// Register state needs to be restored
	registers* registers;

	// The memory mapping, a process can at most 
	//MemoryMapping memoryMappings[MAX_TASK_MEMORY_MB];

	unsigned int    id;          // 
	taskPriority    priority;    // The priority of the task
	unsigned int    timeElapsed; // How long this task has been running since it was switched in
	unsigned int    active;      // Whether this task is currently executing
	taskState       state;       // Current state of task
    task_entry_func entry;       // Entry point of task in memory
    unsigned int    started;     // Whether the task has started
    int             result;      // The return value of the entry function
    char*           name;        // Print friendly name 
} Task;

task_entry_func Task_LoadElf(char* filename, unsigned int addr);
void Task_StartupFunction(Task* task);
Task* Task_Create(task_entry_func, char* name);

#endif