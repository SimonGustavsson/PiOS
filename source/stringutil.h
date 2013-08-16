#include <stdarg.h> // Needed for varying argument length

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

// TODO: Rewrite this to take in the pointer to a buffer where the result will be stored
void itoa(int number, char* buf)
{
	char* result;
	
	// We populate the string backwards, increment to make room for \0
	result++;

	int negative = number < 0;
	if(negative)
	{
		result++;
		number = -number;
	}

	// Find where our string will end
	int shifter = number;
	do
	{
		result++;
		shifter /= 10;
	}while(shifter > 0);

	// Make sure the string is terminated nicely
	*--result = '\0';
	
	// Start converting the digits into characters
	do
	{
		*--result = '0' + (number % 10); // Muahaha!
		number /= 10;
	}while(number > 0);

	if(negative)
		*--result = '-';
		
	// Done!
}

// TODO: Rewrite this to take in the pointer to a buffer where the result will be stored
// const char* printf(char* text, ...)
// {
	// // Set up variable argument list
	// va_list ap;
	// va_start(ap, text);

	// char res[256];
	// char* result = res;
	
	// // Scan all characters in 'text' and look for format specifiers
	// do
	// {
		// if(*text == '%')
		// {
			// if(*(text + 1) == 'd') // Integer (signed)
			// {				
				// char* intStr = strcpy(itoa(va_arg(ap, int)), result);
				
				// result += strlen(intStr);
				
				// // Make sure we skip the type specifier
				// text++;
				
				// continue;
			// }
		// }
		
		// // If we got thus far, it's probably just a normal character
		// *result++ = *text;
	// }while(*text++ != '\0');
	
	// return res;
// }
