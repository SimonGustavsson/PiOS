#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <stdarg.h> // Needed for varying argument length

#define MAX_PRINTF_LENGTH 4096
#define INT_MIN (-2147483647 - 1)
#define INT_MAX 2147483647

int my_strlen(char* str);
char* my_strcpy(const char* src, char* dst);
unsigned int my_strcmp(char* str1, char* str2);
int my_strcmp_s(char* str1, unsigned int size, char* str2);
unsigned int my_strcasecmp(char* str1, char* str2);
void itoa(int number, char* buf);
void utoa(unsigned int number, char* buf);
void dec_to_hex(char* buf, unsigned int dec, unsigned int lowerCase);
void printf(char* text, ...);
void printf_i(char* text, ...);
void printf_s(char* text, unsigned int length, ...);
void vprintf_s(char* text, unsigned int length, va_list args);

// Counts number of digits in an integer
static int int_digit_count(int n) {
    if (n < 0) n = (n == INT_MIN) ? INT_MAX : -n;
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    return 10;
}

int my_sscanf(char* s, const char* format, ...);
int my_sscanf_s(char* s, int sLength, const char* format, ...);
int my_vsscanf(char* s, const char* format, va_list ap);
int my_vsscanf_s(char* s, int sLength, const char* format, va_list ap);

#endif // STRINGUTIL_H
