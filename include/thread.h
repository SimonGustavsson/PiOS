#ifndef THREAD_H
#define THREAD_H

#include "stdbool.h"
#include "stdint.h"
#include "process.h"

#define THREAD_DEFAULT_STACK_SIZE 4096
#define THREAD_STACK_VA_START 0x900000

typedef int(*thread_entry)(char*);

typedef struct {
    uint32_t sprs;
    uint32_t sp;
    uint32_t lr;

    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
    uint32_t lr2;
} thread_regs;

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

typedef struct thread {
    bool isRunning;               // Whether the thread is currently running
    unsigned int timeElapsed;     // How long this task has been running since it was switched in
    thread_priority priority;     // Current priority
    thread_state state;           // Current state of the thread
    Process* owner;               // The process that owns this thread
    char* name;                   // (Optional?) Name of the thread
    thread_regs registers;        // Saved register state
    int32_t result;               // Return value of the threads main function
    thread_entry entry;           // The entry point of the thread
    uint32_t* phyStack;           // Physical stack address (Growing downwards)
    uint32_t* virtStack;          // Virtual stack address (Growing downwards
    uint32_t stackSize;           // Size of the stack
} thread;

thread* thread_create(thread_entry entry);
thread* thread_createWithOwner(thread_entry entry, Process* owner);

void thread_kill(thread*);

#endif