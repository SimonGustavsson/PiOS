#include "task.h"
#include "hardware/mmu.h"

// This gets called when a task is started and is what actually branches
// into the tasks main function
void Task_StartupFunction(Task* task)
{
	// Branch into the tasks main function

	// Do system call to tell the scheduler this task is no longer running
}

Task* Task_Create(void(*mainFunction)(void))
{
	return 0;
}