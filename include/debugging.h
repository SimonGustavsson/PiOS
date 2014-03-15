#ifndef DEBUGGING_H_C
#define DEBUGGING_H_C

#define MAX_FRAME_DEPTH 10

typedef struct {
    int* pc;
    int* lr;
    int* sp;
    int* fp;
} call_frame;

void Debug_PrintCallstack();

#endif