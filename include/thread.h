#include "process.h"
#include "stdbool.h"
#include "stdint.h"

#define THREAD_DEFAULT_STACK_SIZE 4096
#define THREAD_STACK_VA_START 0x1900000

typedef int(*thread_entry)(char*);

typedef struct {
    uint32 sprs;
    uint32 sp;
    uint32 lr;

    uint32 r0;
    uint32 r1;
    uint32 r2;
    uint32 r3;
    uint32 r4;
    uint32 r5;
    uint32 r6;
    uint32 r7;
    uint32 r8;
    uint32 r9;
    uint32 r10;
    uint32 r11;
    uint32 r12;
    uint32 lr2;
} regs;

typedef enum {
    THREAD_PRIO_LOW,
    THREAD_PRIO_MED,
    THREAD_PRIO_HIGH
} thread_priority;

typedef enum {
    // Thread is currently executing
    THREAD_STATE_RUNNING,

    // Thread is in a wait state (Waiting for IO etc..)
    THREAD_STATE_WAITING,

    // Thread is ready to run
    THREAD_STATE_READY
} thread_state;

typedef struct {
    thread_priority priority; // Current priority
    thread_state state;       // Current state of the thread
    Process* owner;           // The process that owns this thread
    char* name;               // (Optional?) Name of the thread
    regs registers;           // Saved register state
    int32_t result;           // Return value of the threads main function
    thread_entry entry;       // The entry point of the thread
    uint32_t* phyStack;       // Physical stack address (Growing downwards)
    uint32_t* virtStack;      // Virtual stack address (Growing downwards
    uint32_t stackSize;       // Size of the stack
} thread;

thread* thread_create(thread_entry entry);
thread* thread_createWithOwner(thread_entry entry, Process* owner);

void thread_kill(thread*);

// TODO: Add and implement in Scheduler
Process* scheduler_get_cur_process(void);
thread* scheduler_get_cur_thread(void);
