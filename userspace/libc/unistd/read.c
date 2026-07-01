#include "unistd.h"

int read(unsigned int fd, void *buf, size_t len)
{
    int ret;

    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(3),
          "b"(fd),
          "c"(buf),
          "d"(len)
        : "memory"
    );

    return ret;
}
