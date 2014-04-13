#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <stdarg.h> // Needed for varying argument length

#define MAX_PRINTF_LENGTH 4096

int my_strlen(char* str);
char* my_strcpy(const char* src, char* dst);
unsigned int my_strcmp(char* str1, char* str2);
int my_strcmp_s(char* str1, unsigned int size, char* str2);
unsigned int my_strcasecmp(char* str1, char* str2);
void itoa(int number, char* buf);
void printf(char* text, ...);
void printf_i(char* text, ...);
void printf_s(char* text, unsigned int length, ...);
void vprintf_s(char* text, unsigned int length, va_list args);

#endif // STRINGUTIL_H
