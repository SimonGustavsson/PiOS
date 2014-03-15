#ifndef TASK_H
#define TASK_H

#include "hardware/mmu.h"

#define MAX_TASK_MEMORY_MB 10
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
	registers registers;

	// The memory mapping, a process can at most 
	MemoryMapping memoryMappings[MAX_TASK_MEMORY_MB];

	unsigned int id;
	taskPriority priority;
	unsigned int timeElapsed;
	unsigned int active;
	taskState state;
} Task;

void task_StartupFunction(Task* task);

#endif TASK_H