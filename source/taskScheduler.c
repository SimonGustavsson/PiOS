#include "taskScheduler.h"
#include "memory.h"
#include "string.h"
#include "util/utilities.h"
#include "hardware/mmu.h"
#include "hardware/timer.h"
#include "queue.h"

// This is really the heart of PiOS - this is where PiOS sits constantly
volatile extern unsigned int gTaskSchedulerTick;
volatile extern unsigned int* get_sp();

static taskScheduler* gScheduler;

void TaskScheduler_Initialize(void)
{
	gScheduler = (taskScheduler*)palloc(sizeof(taskScheduler));
	gScheduler->currentTask = 0;
	gScheduler->tasksRunning = 0;
	gScheduler->tasks.back = 0;
	gScheduler->tasks.front = 0;
	gScheduler->tasks.numNodes = 0;
}

// This is probably not going to be "task" but rather "StartInfo" or similar
void TaskScheduler_EnqueueTask(Task* task)
{
	// Initialize task

	// Add it to the queue for processing
	Queue_Enqueue(&gScheduler->tasks, task);
}

void TaskScheduler_TimerTick(registers* regs)
{
    return;

    //unsigned int shouldSwitchTask = 0;
    //if (gScheduler->currentTask != 0)
    //{
    //    // Increment the amount of time this task has been running
    //    gScheduler->currentTask->timeElapsed += TASK_SCHEDULER_TICK_MS;

    //    // Has it had it's fair share? if so save its state so we can switch it out
    //    if (gScheduler->currentTask->timeElapsed > 2000)
    //    {
    //        shouldSwitchTask = 1;

    //        // Save the current tasks registers to memory
    //        my_memcpy((const void*)&gScheduler->currentTask->registers, (const void*)regs, sizeof(registers));

    //        // Reschedule the task for execution (at the end of the queue)
    //        pqueue_enqueue(&gScheduler->tasks, gScheduler->currentTask);
    //    }
    //}

    //// Nothing is running but we do have tasks waiting - so activate one
    //if (gScheduler->currentTask == 0 && gScheduler->tasks.numNodes > 0)
    //    shouldSwitchTask = 1;

    //if (shouldSwitchTask && gScheduler->tasks.numNodes > 0)
    //{
    //    Task* task = (Task*)pqueue_dequeue(&gScheduler->tasks);

    //    if (task == 0)
    //    {
    //        printf("Woah, woah! I can't switch task to nothing! HALTING!\n");
    //        while (1);
    //    }

    //    gScheduler->currentTask = task;
    //}
}

Task* TaskScheduler_CreateTask(void(*mainFunction)(void))
{
	Task* task = (Task*)palloc(sizeof(Task));

	task->priority = TaskPriorityMedium;

    // Set PC to the task's function so that as soon as we switch modes, that
    // function is invoked
	task->registers.r15 = (unsigned int)&mainFunction;
	task->state = Ready;

	// Allocate frames (start size = 5 MB / task)
    // Ask the mmu for some user pages and store the physical address we get in the tasks 
    // memory mappings so that we can update the page table to point to those physical locations
    // When we swap the task in. (We do this because all tasks share the same virtual memory address space)

	return task;
}