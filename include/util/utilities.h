#include "hardware/uart.h"

#define assert(expression) (!expression ? printf("Assertion failed at %s(%d), expression: " #expression, __FILE__, __LINE__) : (void)0)
#define assert2(expression, message) (!expression ? printf("Assertion failed at %s(%d), expression: " #expression "%s", __FILE__, __LINE__, message) : (void)0)
#define assert3(expression, message, ...) (!(expression) ? printf(message, __VA_ARGS__) : (void)0)
#define assert_uart(expression, message) (!(expression) ? Uart_SendString(message) : (void)0)
#define ReturnOnFailure(result, message) if(((result) != 0)) { printf((char*)message);  goto fExit; }
#define ReturnOnFailureF(result, message, ...) if((result > 0)) { printf((char*)message, __VA_ARGS__); goto fExit; }
#define ReturnOnNull(result, message) if((result) == NULL) { printf((char*)message); goto fExit; }
#define inline __inline

#ifndef UTILITIES_H
#define UTILITIES_H

// Defines to get the VS editor to stop complaining about various GNU stuff
#ifdef _MSC_VER
    #define __attribute__(a)
    #define asm 
#endif

#define S_OK 0
#define E_GENERIC_ERROR -1

// Support for unaligned data access
static inline void write_word(unsigned int val, unsigned char* buf, int offset)
{
    buf[offset + 0] = val & 0xff;
    buf[offset + 1] = (val >> 8) & 0xff;
    buf[offset + 2] = (val >> 16) & 0xff;
    buf[offset + 3] = (val >> 24) & 0xff;
}

static inline unsigned int read_word(unsigned char* buf, int offset)
{
    unsigned int b0 = buf[offset + 0] & 0xff;
    unsigned int b1 = buf[offset + 1] & 0xff;
    unsigned int b2 = buf[offset + 2] & 0xff;
    unsigned int b3 = buf[offset + 3] & 0xff;

    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

static inline unsigned int byte_swap(unsigned int in)
{
    unsigned int b0 = in & 0xff;
    unsigned int b1 = (in >> 8) & 0xff;
    unsigned int b2 = (in >> 16) & 0xff;
    unsigned int b3 = (in >> 24) & 0xff;
    unsigned int ret = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
    return ret;
}

void qsort(void* base, unsigned int nitems, unsigned int element_size, int(*comparer)(const void*, const void*));
void* my_memcpy(const void* dest, const void* src, unsigned int bytesToCopy);
void HexDump(void* mem, unsigned int size);
void reboot(void);

static inline unsigned int byte_to_int(unsigned char* buf)
{
    // NOTE: Assumes little endian
    return (buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24));
}

static inline unsigned int byte_to_short(char* buf)
{
    return (buf[0] + ((int)buf[1] << 8));
}

#endif