#include "ioctl.h"

int ioctl(int fd, unsigned long request, void *arg)
{
    int ret;

    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(54),
          "b"(fd),
          "c"(request),
          "d"(arg)
        : "memory"
    );

    return ret;
}
