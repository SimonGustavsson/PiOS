#include "mem.h"
#include "thread.h"
#include "stddef.h"
#include "hardware/paging.h"
#include "types/string.h"

static bool thread_initStack(thread* t, Process* owner)
{
    int freePage = mem_nextFree();
    if (freePage == -1)
        return false;

    printf("Allocated page stack for thread at 0x%h\n", freePage);

    map_page(owner->ttb0, owner->ttb0_size, (unsigned int)freePage, THREAD_STACK_VA_START,
        PAGE_BUFFERABLE | PAGE_CACHEABLE | PAGE_AP_K_RW_U_RW);

    t->phyStack = (uint32_t*)(freePage + PAGE_SIZE);
    t->virtStack = (uint32_t*)THREAD_STACK_VA_START + PAGE_SIZE;

    return true;
}

thread* thread_create(thread_entry entry)
{
    Process* cur = scheduler_get_cur_process();
    if (cur == NULL)
    {
        printf("Cannot create a thread as no process is running...\n");
        return NULL;
    }
    
    return thread_createWithOwner(entry, cur);
}

thread* thread_createWithOwner(thread_entry entry, Process* owner)
{
    thread* t = (thread*)pcalloc(sizeof(thread), 1);

    t->priority = THREAD_PRIO_MED;
    t->state = THREAD_STATE_READY;
    t->owner = owner;
    t->result = -1;
    t->entry = entry;

    if (!thread_initStack(t, owner))
    {
        printf("Not page available for stack\n");
        phree(t);
    }

    // Initialize registers
    t->registers.sp = (uint32_t)t->virtStack;
    t->registers.r7 = (uint32_t)t->virtStack; // FP! Or is it r11/r12?
    t->registers.sprs = 0x12; // SVC MODE, TODO: User threads?

    return t;
}

void thread_setPriority(thread* t, thread_priority p)
{
    if (t->priority == p)
        return;

    // TODO: Take thread out of current scheduler queue and
    // insert into new one
}

void thread_kill(thread* t)
{
    if (scheduler_get_cur_thread() == t)
    {
        // Thread is currently running
        // Remote it from the scheduler
    }

    // TODO: unmap_page t->phyStack
    mem_free((unsigned int)t->phyStack - PAGE_SIZE);

    if (t->name == NULL)
    {
        phree(t->name);
    }

    phree(t);
}

thread* scheduler_get_cur_thread(void)
{
    return NULL; // TODO: NYI
}

Process* scheduler_get_cur_process(void)
{
    return NULL; // TODO: NYI
}