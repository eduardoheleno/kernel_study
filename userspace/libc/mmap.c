#include "mmap.h"

uintptr_t* mmap(void *addr, size_t len)
{
    uintptr_t *ret;

    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(9),
          "b"(addr),
          "c"(len)
        : "memory"
    );

    return ret;
}
