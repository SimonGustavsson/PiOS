#ifndef DEBUGGING_H_C
#define DEBUGGING_H_C

#define MAX_FRAME_DEPTH 10

void debugPrintCallstack();

typedef struct {
    int* pc;
    int* lr;
    int* sp;
    int* fp;
} call_frame;

#endif