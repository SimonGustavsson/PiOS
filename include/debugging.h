#ifndef DEBUGGING_H_C
#define DEBUGGING_H_C

#define MAX_FRAME_DEPTH 10

typedef struct {
    int* pc;
    int* lr;
    int* sp;
    int* fp;
} call_frame;

typedef struct {
    char* name;
    unsigned int address;
} func_info;

char* Debug_GetClosestPreviousFunction(unsigned int address);
void Debug_ReadFunctionNames(void);
void Debug_PrintCallstack();

#endif