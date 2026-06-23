#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include <stdarg.h>

void printf(const char *s, ...)
{
    va_list ap;
    va_start(ap, s);
    size_t len = strlen(s);
    for (int i = 0; i < len; i++)
    {
        if (s[i] == '%')
        {
            switch (s[i + 1])
            {
                case 'i':
                    int int_param = va_arg(ap, int);
                    char buffer[11];
                    int j = 0;

                    if (int_param == 0) 
                    {
                        write(1, "0", 1);
                        i++;
                        break;
                    }

                    while (int_param > 0) 
                    {
                        buffer[j++] = '0' + (int_param % 10);
                        int_param /= 10;
                    }

                    while (j > 0) 
                    {
                        write(1, &buffer[--j], 1);
                    }
                    i++;
                    break;
                case 's':
                    char *char_param = va_arg(ap, char*);
                    size_t char_len = strlen(char_param);
                    write(1, char_param, char_len);
                    i++;
                    break;
            }
        }
        else
        {
            write(1, &s[i], 1);
        }
    }
}
