#ifndef SCHEDULER_H
#define SCHEDULER_H

#define TASK_SCHEDULER_TICK_MS 2000
#include "process.h"
#include "types/queue.h"

typedef struct {
	Process* currentTask; // TODO: should be threads
	Queue tasks;
} taskScheduler;

void Scheduler_Initialize(void);
void Scheduler_Start(void);

// Switches to the next task in line Save registers and call this function from assembly
void Scheduler_TimerTick(registers* registers);

// Loads the given elf and enqueues it
void Scheduler_Enqueue(Process* p);

unsigned int Scheduler_GetNextTID(void);
// Switches to the next task in the list
void Scheduler_NextTask(registers* regs);
void Scheduler_StartTask(Process* p);

#endif