#include "munmap.h"

int munmap(void* addr, size_t len)
{
    int ret;

    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(91),
          "b"(addr),
          "c"(len)
        : "memory"
    );

    return ret;
}
