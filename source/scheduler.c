#include "util/utilities.h"
#include "asm.h"
#include "main.h"
#include "mem.h"
#include "memory.h"
#include "memory_map.h"
#include "scheduler.h"
#include "stddef.h"
#include "hardware/interrupts.h"
#include "hardware/mmu_c.h"
#include "hardware/timer.h"
#include "types/string.h"
#include "types/queue.h"
#include "thread.h"
#include "process.h"
#include "thread.h"

static taskScheduler* gScheduler;
static unsigned int gNextTID;

void Scheduler_PrintRegs(thread_regs* regs)
{
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("0x%h r0: 0x%h\n", &regs->r0, regs->r0);
    printf("0x%h r1: 0x%h\n", &regs->r1, regs->r1);
    printf("0x%h r2: 0x%h\n", &regs->r2, regs->r2);
    printf("0x%h r3: 0x%h\n", &regs->r3, regs->r3);
    printf("0x%h r4: 0x%h\n", &regs->r4, regs->r4);
    printf("0x%h r5: 0x%h\n", &regs->r5, regs->r5);
    printf("0x%h r6: 0x%h\n", &regs->r6, regs->r6);
    printf("0x%h r7: 0x%h\n", &regs->r7, regs->r7);
    printf("0x%h r8: 0x%h\n", &regs->r8, regs->r8);
    printf("0x%h r9: 0x%h\n", &regs->r9, regs->r9);
    printf("0x%h r10: 0x%h\n", &regs->r10, regs->r10);
    printf("0x%h r11: 0x%h\n", &regs->r11, regs->r11);
    printf("0x%h r12: 0x%h\n", &regs->r12, regs->r12);
    printf("0x%h LR: 0x%h\n", &regs->lr, regs->lr);

    printf("0x%h lr2: 0x%h\n", &regs->lr2, regs->lr2);
    printf("0x%h SPRS: 0x%h\n", &regs->sprs, regs->sprs);
    printf("0x%h SP: 0x%h\n", &regs->sp, regs->sp);
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

void Scheduler_Initialize(void)
{
	gScheduler = (taskScheduler*)palloc(sizeof(taskScheduler));
	gScheduler->currentThread = NULL;
    gScheduler->threads.back = NULL;
    gScheduler->threads.front = NULL;
    gScheduler->threads.numNodes = 0;

    gNextTID = 1;
}

void Scheduler_Start(void)
{
    Timer_Clear();
    Timer_SetInterval(TASK_SCHEDULER_TICK_MS);
    Arm_IrqEnable(interrupt_source_system_timer);

    // Switch in first task?
}

unsigned int Scheduler_GetNextTID(void)
{
    // For now: Just an incremental integer, need to do someting better in the future
    return gNextTID++;
}

// This is obsolete, don't use it? Should enqueue threads instead
void Scheduler_Enqueue(Process* task)
{
	// Add it to the queue for processing
	Queue_Enqueue(&gScheduler->threads, task->mainThread);
}

void Scheduler_NextTask(thread_regs* reg)
{
    //printf("Scheduler - cpsr is: 0x%h\n", reg->sprs);

    unsigned int shouldSwitchTask = 0;
    thread* cur = gScheduler->currentThread;
    if (cur != NULL && gScheduler->threads.numNodes > 0)
    {
        //printf("Scheduler: Have a current running task!\n");

        cur->timeElapsed += TASK_SCHEDULER_TICK_MS;

        // Has this task had it's fair share?
        if (cur->timeElapsed >= (TASK_SCHEDULER_TICK_MS * 2))
        {
            //printf("Scheduler: Switching out %s\n", cur->name);

            shouldSwitchTask = 1;

            printf("Old threads SP: 0x%h\n", reg->sp);

            // Save registers
            my_memcpy(&cur->registers, reg, sizeof(thread_regs));

            //printf("saved registers, sp: 0x%h\n", cur->registers.sp);

            cur->state = TS_Ready;
            cur->isRunning = false;

            // Put it back in the queue
            Queue_Enqueue(&gScheduler->threads, cur);
        }
    }
    else if (cur == NULL)
    {
        //printf("Scheduler: First run? Switch to a task!\n");
        shouldSwitchTask = 1;
    }

    if (!shouldSwitchTask || gScheduler->threads.numNodes == 0)
        return;

    //printf("Registers before: \n");
    //Scheduler_PrintRegs(reg);

    thread* next = (thread*)Queue_Dequeue(&gScheduler->threads);

    if (next == NULL)
    {
        printf("Failed to retrieve next thread, how can it be null!? Skippin CTX switch...\n");
        return;
    }

    //printf("Scheduler: Switching in %s\n", next->name);

    next->isRunning = true;
    next->state = TS_Running;
    gScheduler->currentThread = next;

    //printf("Scheduler - Setting SPSR to: 0x%h\n", next->registers.sprs);
    //printf("Updating TTBC, Size: %d\n", next->ttb0_size);

    // Update TTCR
    unsigned int ttbc = get_ttbc();
    ttbc = next->owner->ttb0_size;
    set_ttbc(ttbc);
    //printf("Set ttbc to: 0x%h\n", ttbc);

    // Switch in the process' TT
    //printf("Switch to task's TTB0, ttb0 addr (Physical): 0x%h\n", next->ttb0_physical);
    set_ttb0(next->owner->ttb0_physical, 1);

    for (int i = 0; i < 1000; i++);

    // Restore the tasks registers
    my_memcpy(reg, &next->registers, sizeof(thread_regs));

    printf("\n* * Switching in thread '%s' in process %s * *\n", next->name, next->owner->name);
    //Scheduler_PrintRegs(reg);
}

void Scheduler_TimerTick(thread_regs* regs)
{
    // A switch isn't necessary
    if (gScheduler->threads.numNodes == 0 && gScheduler->currentThread != NULL)
    {
        printf("Scheduler: Not enough threads running to bother...\n");
    }
    else
    {
        Scheduler_NextTask(regs);
    }
    
    // Restart the timer
    Timer_Clear();
    Timer_SetInterval(TASK_SCHEDULER_TICK_MS * 2);
    Arm_IrqEnable(interrupt_source_system_timer); // Don't think I have to do this?
}

thread* Scheduler_GetCurrentThread(void)
{
    return gScheduler->currentThread;
}