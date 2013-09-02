#include "stringutil.h"
#include "terminal.h"

int strlen(char* str)
{
	int length = 0;
	while(*str++)
		length++;

	return length;
}

char* strcpy(const char* src, char* dst)
{
	char *ptr;
	ptr = dst;
	while((*dst++ = *src++));

	return ptr;
}

char ctolower(char c)
{
	if(c >= 'A' && c <= 'Z')
		return 'a' + (c - 'A');
	else
		return c;
}

unsigned int strcasecmp(char* str1, char* str2)
{
	for(;; str1++, str2++)
	{
		int d = ctolower(*str1) - ctolower(*str2); 
		if(d != 0 || !*str1)
			return d;
	}
}

unsigned int strcmp(char* str1, char* str2)
{
	unsigned int len1 = strlen(str1);
	unsigned int len2 = strlen(str2);
	
	if(len1 != len2)
		return 0; // Not equal
		
	unsigned int i;
	for(i = 0; i < len1; i++)
		if(str1[i] != str2[i])
			return 0;
			
	// They must be equal
	return 1;
}

void itoa(int number, char* buf)
{
	// We populate the string backwards, increment to make room for \0
	buf++;

	int negative = number < 0;
	if(negative)
	{
		buf++;
		number = -number;
	}

	// Find where our string will end
	int shifter = number;
	do
	{
		buf++;
		shifter /= 10;
	}while(shifter > 0);

	// Make sure the string is terminated nicely
	*--buf = '\0';
	
	// Start converting the digits into characters
	do
	{
		*--buf = '0' + (number % 10); // Muahaha!
		number /= 10;
	}while(number > 0);

	if(negative)
		*--buf = '-';
}

void printf(char* text, ...)
{
	va_list ap;
	va_start(ap, text);

	char res[256];
	char* result = res;
	
	// scan all characters in the string and look for format specifiers
	do
	{
		if(*text == '%')
		{
			if(*(text + 1) == 'c') // unsigned char
			{
				*result++ = (char)va_arg(ap, unsigned int);
			}
			else if(*(text + 1) == 'd') // integer (signed)
			{
				char itoBuf[10];
				itoa(va_arg(ap, int), &itoBuf[0]);
				
				char* intstr = strcpy(itoBuf, result);
				
				result += strlen(intstr);
			}
			else if(*(text + 1) == 's') // string
			{
				char* arg = (char*)va_arg(ap, int);
				
				strcpy(arg, result);
				
				result += strlen(arg);
			}
			
			// make sure we skip the type specifier
			text++;
			
			continue;
		}
		
		// if we got this far, it's probably just a normal character
		*result++ = *text;
	}while(*text++ != '\0');
	
	print(res, strlen(res));
}
