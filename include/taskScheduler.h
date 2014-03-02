#ifndef SCHEDULER_H
#define SCHEDULER_H

#define TASK_SCHEDULER_TICK_MS 200
#define MAX_NUM_TASKS 10
#include "task.h"
#include "pqueue.h"

typedef struct {
	Task* currentTask;
	Queue tasks;
	unsigned int tasksRunning;
} taskScheduler;

// Switches to the next task in line
// Save registers and call this function from assembly
void taskScheduler_TimerTick(registers* registers);
void taskScheduler_Init(void);
Task* taskScheduler_CreateTask(void(*mainFunction)(void));
void taskScheduler_EnqueueTask(Task* task);

#endif SCHEDULER_H
