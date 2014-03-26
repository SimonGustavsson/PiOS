#include "types/string.h"
#include "terminal.h"

int my_strlen(char* str)
{
	int length = 0;
	while(*str++)
		length++;

	return length;
}

char* my_strcpy(const char* src, char* dst)
{
	char *ptr;
	ptr = dst;
	while((*dst++ = *src++));

	return ptr;
}

char my_ctolower(char c)
{
	if(c >= 'A' && c <= 'Z')
		return 'a' + (c - 'A');
	else
		return c;
}

unsigned int my_strcasecmp(char* str1, char* str2)
{
	for(;; str1++, str2++)
	{
		int d = my_ctolower(*str1) - my_ctolower(*str2); 
		if(d != 0 || !*str1)
			return d;
	}
}

int my_strcmp_s(char* str1, unsigned int size, char* str2)
{
    unsigned int i;
    for (i = 0; i < size; i++)
    {
        if (*str1++ != *str2++)
            return -1;
    }

    return 0;
}

unsigned int my_strcmp(char* str1, char* str2)
{
	unsigned int len1 = my_strlen(str1);
	unsigned int len2 = my_strlen(str2);
    
	if(len1 != len2)
		return 0; // Not equal
		
	unsigned int i;
    for (i = 0; i < len1; i++)
    {
        if (str1[i] != str2[i])
            return 0;
    }

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

void dec_to_hex(char* buf, unsigned int dec)
{
	unsigned int reminder[50];
	unsigned int length = 0;
	char* buf_ptr = buf;

    if (dec == 0)
    {
        *buf_ptr++ = '0';
    }

	while(dec > 0)
	{
		reminder[length] = dec % 16;
		dec = dec / 16;	
		length++;
	}


	int i;
	for(i = length - 1; i >= 0; i--)
	{
		switch(reminder[i])
		{
			case 10:
				*buf_ptr++ = 'A';
				break;
			case 11:
				*buf_ptr++ = 'B';
				break;
			case 12:
				*buf_ptr++ = 'C';
				break;
			case 13:
				*buf_ptr++ = 'D';
				break;
			case 14:
				*buf_ptr++ = 'E';
				break;
			default:
				{
					// Display as digits
					char itoa_buf[10];
					itoa(reminder[i], itoa_buf);

					my_strcpy(itoa_buf, buf_ptr);

					buf_ptr += my_strlen(itoa_buf);
				break;
				}
		}
	}

	*buf_ptr = '\0';
}

void printf_i(char* text, ...)
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
				
				char* intstr = my_strcpy(itoBuf, result);
				
				result += my_strlen(intstr);
			}
			else if(*(text + 1) == 's') // string
			{
				char* arg = (char*)va_arg(ap, int);
				
				my_strcpy(arg, result);
				
				result += my_strlen(arg);
			}
			else if(*(text + 1) == 'h') // hex
			{
				char hex_buf[50];
				char* hex_buf_ptr = &hex_buf[0];

				dec_to_hex(hex_buf_ptr, va_arg(ap, int));
				
				my_strcpy(hex_buf, result);
				
				unsigned int hexLength = my_strlen(hex_buf);

				result += hexLength;
			}
			
			// make sure we skip the type specifier
			text++;
			
			continue;
		}
		
		// if we got this far, it's probably just a normal character
		*result++ = *text;
	}while(*text++ != '\0');
	
	Terminal_PrintImportant(res, my_strlen(res));
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
				
				char* intstr = my_strcpy(itoBuf, result);
				
				result += my_strlen(intstr);
			}
			else if(*(text + 1) == 's') // string
			{
				char* arg = (char*)va_arg(ap, int);
				
				my_strcpy(arg, result);
				
				result += my_strlen(arg);
			}
			else if(*(text + 1) == 'h') // hex
			{
				char hex_buf[50];
				char* hex_buf_ptr = &hex_buf[0];

				dec_to_hex(hex_buf_ptr, va_arg(ap, int));
				
				my_strcpy(hex_buf, result);
				
				unsigned int hexLength = my_strlen(hex_buf);

				result += hexLength;
			}

			// make sure we skip the type specifier
			text++;
			
			continue;
		}
		
		// if we got this far, it's probably just a normal character
		*result++ = *text;
	}while(*text++ != '\0');
	
	Terminal_Print(res, my_strlen(res));
}
