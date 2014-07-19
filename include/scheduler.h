#ifndef SCHEDULER_H
#define SCHEDULER_H

#define TASK_SCHEDULER_TICK_MS 2000
#include "types/queue.h"
#include "process.h"
#include "thread.h"

typedef struct {
    thread* currentThread;
	Queue threads;
} taskScheduler;

void Scheduler_Initialize(void);
void Scheduler_Start(void);

// Switches to the next task in line Save registers and call this function from assembly
void Scheduler_TimerTick(thread_regs* registers);

// Loads the given elf and enqueues it
void Scheduler_Enqueue(Process* p);

unsigned int Scheduler_GetNextTID(void);
// Switches to the next task in the list
void Scheduler_NextTask(thread_regs* regs);
void Scheduler_StartTask(Process* p);
thread* Scheduler_GetCurrentThread(void);

#endif