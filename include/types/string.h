#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <stdarg.h> // Needed for varying argument length

int my_strlen(char* str);
char* my_strcpy(const char* src, char* dst);
unsigned int my_strcmp(char* str1, char* str2);
int my_strcmp_s(char* str1, unsigned int size, char* str2);
unsigned int my_strcasecmp(char* str1, char* str2);
void itoa(int number, char* buf);
void printf(char* text, ...);
void printf_i(char* text, ...);

#endif // STRINGUTIL_H
