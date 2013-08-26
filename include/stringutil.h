#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <stdarg.h> // Needed for varying argument length

int strlen(char* str);
char* strcpy(const char* src, char* dst);
void itoa(int number, char* buf);
void printf(char* text, ...);

#endif // STRINGUTIL_H
