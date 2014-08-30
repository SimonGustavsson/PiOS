#include "types/string.h"
#include "terminal.h"

#define INT_MIN (-2147483647 - 1)
#define INT_MAX 2147483647

//Counts number of digits in an integer
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

static int my_vsscanf_core(char* s, int sLength, const char* format, va_list ap, int useLength);

int strchrmatch(char* str, unsigned int str_len, char c)
{
    int count = -1;
    unsigned int i;
    for (i = 0; i <= str_len; i++)
    {
        if (str[i] == c)
            count++;
    }

    return count;
}

unsigned int atoi(char* str)
{
    unsigned int result = 0;
    while (*str) {
        result = (result << 3) + (result << 1) + (*str) - '0';
        str++;
    }

    return result;
}

int my_strlen(char* str)
{
	int length = 0;
	while(*str++)
		length++;

	return length;
}

char* my_strcpy_s(const char* src, unsigned int dest_len, char* dst)
{
    char* ptr;
    ptr = dst;
    unsigned int num_written = 0;
    while ((*dst++ = *src++) && (num_written++ < dest_len));

    return ptr;
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

void utoa(unsigned int number, char* buf)
{
    // Populating backwards
    buf++;
    unsigned int shifter = number;
    do
    {
        buf++;
        shifter /= 10;
    } while (shifter > 0);

    // Make sure the string is terminated nicely
    *--buf = '\0';

    // Start converting the digits into characters
    if (number == 0)
        *--buf = '0';
    else
    {

        do
        {
            *--buf = '0' + (number % 10); // Muahaha!
            number /= 10;
        } while (number > 0);
    }
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

void dec_to_hex(char* buf, unsigned int dec, unsigned int lowerCase)
{
    unsigned int reminder[50];
    unsigned int length = 0;
    char* buf_ptr = buf;

    if (dec == 0)
    {
        *buf_ptr++ = '0';
    }

    while (dec > 0)
    {
        reminder[length] = dec % 16;
        dec = dec / 16;
        length++;
    }

    if (lowerCase)
        lowerCase = 32;

    int i;
    for (i = length - 1; i >= 0; i--)
    {
        switch (reminder[i])
        {
        case 0:
            *buf_ptr++ = '0';
            break;
        case 10:
            *buf_ptr++ = 'A' + lowerCase;
            break;
        case 11:
            *buf_ptr++ = 'B' + lowerCase;
            break;
        case 12:
            *buf_ptr++ = 'C' + lowerCase;
            break;
        case 13:
            *buf_ptr++ = 'D' + lowerCase;
            break;
        case 14:
            *buf_ptr++ = 'E' + lowerCase;
            break;
        case 15:
            *buf_ptr++ = 'F' + lowerCase;
            break;
        case 16:
            *buf_ptr++ = '1';
            *buf_ptr++ = '0';
            break;
        default:
            {
                // Display as digits
                char itoa_buf[10];
                //itoa(reminder[i], itoa_buf);
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

				dec_to_hex(hex_buf_ptr, va_arg(ap, int), 0);
				
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
    va_list args;
    va_start(args, text);


    vprintf_s(text, 2048, args);
}

void printf_s(char* text, unsigned int length, ...)
{
    va_list args;
    va_start(args, length);

    vprintf_s(text, length, args);
}

void vprintf_s(char* text, unsigned int length, va_list ap)
{
    char res[MAX_PRINTF_LENGTH];
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
				utoa(va_arg(ap, unsigned int), &itoBuf[0]);
				
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

				dec_to_hex(hex_buf_ptr, va_arg(ap, int), 0);
				
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

int my_sscanf(char* s, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    int retVal = my_vsscanf_core(s, -1, format, args, 0);

    va_end(args);

    return retVal;
}

int my_vsscanf(char* s, const char* format, va_list ap)
{
    return my_vsscanf_core(s, -1, format, ap, 0);
}

int my_sccanf_s(char* s, int sLength, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    int retVal = my_vsscanf_core(s, sLength, format, args, 1);

    va_end(args);

    return retVal;
}

int my_vsscanf_s(char* s, int sLength, const char* format, va_list ap)
{
    return my_vsscanf_core(s, sLength, format, ap, 1);
}

static int my_vsscanf_core(char* s, int sLength, const char* format, va_list ap, int useLength)
{
    /* TODO: Flags
    - : Left justify (wut?)
    + : Preceed number with + or -
    # : Used with x and X to print 0x, for a,e,f,g print comma
    */

    char* res = s;
    unsigned int width = 0;
    int readWidth = 0;
    char* curArg = 0;
    int argsRead = 0;
    int resLen = 0; // for _s

    do
    {
        char cur = *format;
        char next = *(format + 1);
        if (cur != '%' && readWidth != 1)
        {
            *res++ = cur;
            resLen++; // for _s
        }
        else
        {
            // If  we were reading length and reached the end of the length
            if (readWidth)
            {
                next = cur;
            }

            switch (next)
            {
            case 'c': // Unsigned Character
            {
                char charArg = (unsigned char)va_arg(ap, int);
                argsRead++;

                *res++ = charArg;
                resLen++; // for _s

                format++; // As this was a format specifier, make sure we skip 2

                break;
            }
            case 'd': // Signed decimal int
            {
                int arg = va_arg(ap, int);
                argsRead++;
                char ds[12];

                // TODO: itoa needs to return the number of digits instead of using int_digit_count
                itoa(arg, &ds[0]);
                int argDigitCount = int_digit_count(arg);

                // Copy it over to result
                int maxLen = width > 0 ? width : argDigitCount;

                int i;
                for (i = 0; i < maxLen; i++)
                    *res++ = ds[i];

                resLen += maxLen; // for _s

                if (width == 0)
                    format++; // As this was a format specifier, make sure we skip 2

                // Done with this width specifier
                width = 0;

                break;
            }
            case 's': // String
            {
                curArg = (char*)va_arg(ap, int);

                // Invalid string, cancel out before we attempt to read a null pointer
                if (curArg == 0)
                    return argsRead;

                argsRead++;

                int arg_chars_read = 0;
                do
                {
                    *res++ = *curArg++;
                    arg_chars_read++;
                    resLen++; // for _s
                } while (*curArg != 0 && (width == 0 || arg_chars_read < width));

                if (width == 0)
                    format++;

                // Done with this width specifier
                width = 0;

                break;
            }
            case 'x': // unsigned hexadecimal int
            case 'X': // unsigned hexadecimal int (uppercase)
            case 'p': // Pointer - Write hex address
            case 'P': // Pointer - Write hex address (uppercase)
            {
                unsigned int lowerCase = next == 'x' || next == 'p';

                int hexArg = va_arg(ap, int);
                argsRead++;

                char hexStr[9];
                dec_to_hex(hexStr, hexArg, lowerCase);

                if (width > 0)
                {
                    int zeroesToPad = width - my_strlen(hexStr);

                    // Hex string limit of 8 characters (32-bit)
                    if (zeroesToPad > 8 - my_strlen(hexStr))
                        zeroesToPad = 8 - my_strlen(hexStr);

                    if (zeroesToPad > 0)
                    {
                        // Shift the entire string over
                        unsigned int i, j;
                        j = 0;
                        for (i = 8 - zeroesToPad; i > 0; i--)
                        {
                            // Fix this nasty shit
                            hexStr[zeroesToPad + i - 1] = hexStr[8 - zeroesToPad - j - 1];
                            j++;
                        }

                        // Insert zeroes
                        for (i = 0; i < zeroesToPad; i++)
                            hexStr[i] = '0';

                        hexStr[8] = 0; // Nulterminate
                    }
                }
                
                // Pointer - Prefix with 0x
                if (next == 'p' || next == 'P')
                {
                    *res++ = '0';
                    *res++ = 'x';
                }

                my_strcpy(&hexStr[0], res);
                resLen += my_strlen(hexStr); // for _s
                res += my_strlen(hexStr);

                // Format specifier, skip 2
                if (width == 0)
                    format++;

                // Done with this width specifier
                width = 0;

                break;
            }
            case 'f': // TODO: Decimal float
                break;
            case 'a': // TODO: Hexadecimal float
                break;
            case 'A': // TODO: Hexadecimal float (uppercase)
                break;
            case '%': // Escaped %
                *res++ = cur;
                format += 2;
                break;
            case '*': // Width specified in arg
            {
                int widthArg = va_arg(ap, int);
                argsRead++;

                width = widthArg;

                // for _s version
                if (useLength && width > sLength)
                {
                    printf("Invalid width arg passed to scanf, arg %d max %d\n", width, sLength);
                    return -1;
                }

                format++;
                readWidth = 1;

                continue;
            }
            default:
                // Length specifier?
                if (next >= '0' && next <= '9')
                {
                    format++; // Skip past the % and read the length
                    do
                    {
                        width = (width * 10) + (next - '0');

                        // for _s versions
                        if (useLength && width > sLength)
                            return -1; // Invalid
                        format++;
                        next = *format;
                    } while (next >= '0' && next <= '9');

                    readWidth = 1;

                    // Rewind so we can read the format type
                    format--;

                    continue;
                }

                break;
            }
        }

        // We've handled one arg after reading the length specifier, reset flag
        readWidth = 0;
    } while (*++format != 0 && (useLength ? resLen < sLength : 1));

    // Null-terminate% the string
    *res = 0;

    return argsRead;
}
