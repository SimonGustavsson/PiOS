#include "taskScheduler.h"
#include "memory.h"
#include "types/string.h"
#include "util/utilities.h"
#include "hardware/mmu_c.h"
#include "hardware/timer.h"
#include "types/queue.h"
#include "hardware/interrupts.h"
#include "stddef.h"
#include "memory_map.h"
#include "asm.h"
#include "mem.h"

// This is really the heart of PiOS - this is where PiOS sits constantly

static taskScheduler* gScheduler;
static unsigned int gNextTID;

void TaskScheduler_Initialize(void)
{
	gScheduler = (taskScheduler*)palloc(sizeof(taskScheduler));
	gScheduler->currentTask = 0;
	gScheduler->tasksRunning = 0;
	gScheduler->tasks.back = 0;
	gScheduler->tasks.front = 0;
	gScheduler->tasks.numNodes = 0;

    gNextTID = 1;
}

void TaskScheduler_Start(void)
{
    Timer_Clear();
    Timer_SetInterval(TASK_SCHEDULER_TICK_MS);
    Arm_IrqEnable(interrupt_source_system_timer);

    // Switch in first task?
}

unsigned int TaskScheduler_GetNextTID(void)
{
    // For now: Just an incremental integer, need to do someting better in the future
    return gNextTID++;
}

// This is probably not going to be "task" but rather "StartInfo" or similar
void TaskScheduler_Enqueue(Task* task)
{
	// Add it to the queue for processing
	Queue_Enqueue(&gScheduler->tasks, task);

    gScheduler->tasksRunning++;
}

void TaskScheduler_NextTask(void)
{
    Task* old = gScheduler->currentTask;
    if (old != 0)
    {
        // Switch out the old task
    }


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

    // if the new task's "started" value == 0, call Task_StartupFunction()
}

void TaskScheduler_StartTask(Task* task)
{
    printf("Starting task '%s', Id: %d, ttb0: 0x%h, Priority: %d. Path: %s\n",
        task->name, task->id, task->ttb0, task->priority, task->path);

    // Activate the translation table for the task
    set_ttb0(task->ttb0, 1);

    // Update the Translation Table Control register with the task's TT's size
    unsigned int ttbc = get_ttbc();
    ttbc &= task->ttb0_size;
    set_ttbc(ttbc);

    // Jump to the startup procedure of the task
    task->result = task->entry();

    // TODO: Remove task from scheduler
}

void TaskScheduler_TimerTick(registers* regs)
{
    printf("Task scheduler tick!\n");

    // Restart the timer
    Timer_Clear();
    Timer_SetInterval(TASK_SCHEDULER_TICK_MS);

    //TaskScheduler_NextTask();
}

Task* TaskScheduler_CreateTask(void(*mainFunction)(void))
{
	Task* task = (Task*)palloc(sizeof(Task));

	task->priority = TaskPriorityMedium;

    // Set PC to the task's function so that as soon as we switch modes, that
    // function is invoked
	task->registers->r15 = (unsigned long)&mainFunction;
	task->state = Ready;

	// Allocate frames (start size = 5 MB / task)
    // Ask the mmu for some user pages and store the physical address we get in the tasks 
    // memory mappings so that we can update the page table to point to those physical locations
    // When we swap the task in. (We do this because all tasks share the same virtual memory address space)

	return task;
}