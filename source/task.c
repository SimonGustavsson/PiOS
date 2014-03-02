#include "task.h"
#include "mmu.h"

// This gets called when a task is started and is what actually branches
// into the tasks main function
void task_StartupFunction(Task* task)
{
	// Branch into the tasks main function

	// Do system call to tell the scheduler this task is no longer running
}

Task* task_Create(void(*mainFunction)(void))
{
	return 0;
}