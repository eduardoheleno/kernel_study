#include "unistd.h"

int main(void)
{
    write(1, "Executing compiled flat binary with custom libc\n", 48);
}
