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
    unsigned int address;
    unsigned int name_len;
    char* name;
} function_name;

void Debug_ReadFunctionNames(void);
void Debug_PrintCallstack();

#endif