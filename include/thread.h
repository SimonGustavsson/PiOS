#include "process.h"
#include <stdbool.h>

typedef struct {
    unsigned int r0;
    unsigned int r1;
    unsigned int r2;
    unsigned int r3;
    unsigned int r4;
    unsigned int r5;
    unsigned int r6;
    unsigned int r7;
    unsigned int r8;
    unsigned int r9;
    unsigned int r10;
    unsigned int r11;
    unsigned int r12;
    unsigned int r13; // SP
    unsigned int r14; // LR
    unsigned int r15; // PC
} regs;

typedef enum {
    thread_low,
    thread_normal,
    thread_high
} thread_priority;

typedef enum {
    running,
    waiting,
    ready
} thread_state;

typedef struct {
    thread_priority priority;
    thread_state state;
    Process* owner;           // The process that owns this thread
    char* name;
    regs registers;           // Saved register state
    int result;
} thread;

thread* thread_create();

thread* thread_delete();