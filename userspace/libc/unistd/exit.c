#include "unistd.h"

void exit(void)
{
    __asm__ volatile (
        "mov $1, %%eax\n"
        "int $0x80\n"
        :
        :
        : "eax"
    );
}
