#include "unistd.h"

int write(unsigned int fd, const void *buf, size_t len)
{
    int ret;

    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(4),
          "b"(fd),
          "c"(buf),
          "d"(len)
    );

    return ret;
}
