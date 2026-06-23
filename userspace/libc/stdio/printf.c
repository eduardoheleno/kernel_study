#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include <stdarg.h>

void printf(const char *s, ...)
{
    va_list ap;
    va_start(ap, s);
    size_t len = strlen(s);

    char formatted_buffer[len];
    for (int i = 0; i < len; i++)
    {
        if (s[i] == '%')
        {
            switch (s[i + 1])
            {
                case 'i':
                    int value = va_arg(ap, int);
                    char buffer[11];
                    int j = 0;

                    if (value == 0) 
                    {
                        formatted_buffer[i++] = '0';
                        break;
                    }

                    while (value > 0) 
                    {
                        buffer[j++] = '0' + (value % 10);
                        value /= 10;
                    }

                    while (j > 0) 
                    {
                        formatted_buffer[i++] = buffer[--j];
                    }
                    break;
            }
        }
        else
        {
            formatted_buffer[i] = s[i];
        }
    }

    size_t formatted_buffer_len = strlen(formatted_buffer);
    write(1, formatted_buffer, formatted_buffer_len);
}
