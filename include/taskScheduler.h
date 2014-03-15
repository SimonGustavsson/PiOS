#ifndef SCHEDULER_H
#define SCHEDULER_H

#define TASK_SCHEDULER_TICK_MS 200
#define MAX_NUM_TASKS 10
#include "task.h"
#include "queue.h"

typedef struct {
	Task* currentTask;
	Queue tasks;
	unsigned int tasksRunning;
} taskScheduler;

// Switches to the next task in line
// Save registers and call this function from assembly
void TaskScheduler_TimerTick(registers* registers);
void TaskScheduler_Initialize(void);
Task* TaskScheduler_CreateTask(void(*mainFunction)(void));
void TaskScheduler_EnqueueTask(Task* task);

#endif SCHEDULER_H
