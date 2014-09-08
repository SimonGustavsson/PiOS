#include "mem.h"
#include "thread.h"
#include "stddef.h"
#include "hardware/mmu.h"
#include "types/string.h"
#include "process.h"
#include "scheduler.h"
#include "util/memutil.h"
#include "stdint.h"

static bool thread_initStack(thread* t, Process* owner)
{
    int freePage0 = mem_nextFree();
    if (freePage0 == -1)
        return false;

    int freePage1 = mem_nextFree();
    if(freePage1 == -1)
    {
        mem_free(freePage0);
        return false;
    }

    //printf("Initializing thread stack at 0x%h (phys: 0x%h)\n", THREAD_STACK_VA_START, freePage);

    map_kpage(owner->ttb0, owner->ttb0_size, (unsigned int)freePage0, THREAD_STACK_VA_START - PAGE_SIZE);

    map_kpage(owner->ttb0, owner->ttb0_size, (unsigned int)freePage1, (THREAD_STACK_VA_START - (PAGE_SIZE * 2)));

    t->stackPages = (uint32_t*)pcalloc(THREAD_MAX_PAGES, sizeof(int));
    t->stackPages[0] = freePage0;
    t->stackPages[1] = freePage1;
    t->nStackPages = 2;

    t->virtStack = (uint32_t*)(THREAD_STACK_VA_START);

    //printf("Virtual stack: 0x%h - Physical stack: 0x%h\n", t->virtStack, t->phyStack);

    return true;
}

thread* thread_create(thread_entry entry)
{
    //printf("Creating a new thread with entry 0x%h\n", entry);

    thread* currentThread = Scheduler_GetCurrentThread();

    if (currentThread == NULL)
    {
        printf("Cannot create a thread without an owner.\n");
        return NULL;
    }

    Process* cur = currentThread->owner;
    if (cur == NULL)
    {
        printf("Cannot create a thread as no process is running...\n");
        return NULL;
    }
    
    return thread_createWithOwner(entry, cur);
}

thread* thread_createWithOwner(thread_entry entry, Process* owner)
{
    //printf("Creating thread with owner %s\n", owner->name);

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
    //printf("Creating thread, virtual SP: 0x%h, entry point: 0x%h\n", t->virtStack, entry);
    t->registers.r0 = 1;
    t->registers.r1 = 2;
    t->registers.sp = (uint32_t)t->virtStack;
    t->registers.r7 = (uint32_t)t->virtStack; // FP! Or is it r11/r12?
    t->registers.r12 = (uint32_t)t->virtStack;
    t->registers.sprs = 0x53; // SVC MODE, TODO: User threads?
    t->registers.lr2 = (uint32_t)entry;

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
    if (Scheduler_GetCurrentThread() == t)
    {
        // Thread is currently running
        // Remote it from the scheduler
    }

    // TODO: unmap_page t->phyStack
    for(int i = 0; i < THREAD_MAX_PAGES; i++)
        mem_free(t->stackPages[i]);

    phree(t->stackPages);

    if (t->name == NULL)
    {
        phree(t->name);
    }

    phree(t);
}