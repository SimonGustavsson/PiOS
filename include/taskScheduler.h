#ifndef SCHEDULER_H
#define SCHEDULER_H

#define TASK_SCHEDULER_TICK_MS 200
#define MAX_NUM_TASKS 10
#include "task.h"
#include "types/queue.h"

typedef struct {
	Task* currentTask;
	Queue tasks;
	unsigned int tasksRunning;
} taskScheduler;

void TaskScheduler_Initialize(void);
void TaskScheduler_Start(void);

// Switches to the next task in line Save registers and call this function from assembly
void TaskScheduler_TimerTick(registers* registers);
Task* TaskScheduler_CreateTask(void(*mainFunction)(void));
void TaskScheduler_EnqueueTask(Task* task);
unsigned int TaskScheduler_GetNextTID(void);
// Switches to the next task in the list
void TaskScheduler_NextTask(void);


#endif