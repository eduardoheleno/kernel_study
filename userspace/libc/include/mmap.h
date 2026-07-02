#ifndef _MMAP_H
#define _MMAP_H

#include <stdint.h>
#include <stddef.h>

uintptr_t* mmap(void *addr, size_t len);

#endif
