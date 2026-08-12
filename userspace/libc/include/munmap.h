#ifndef _MUNMAP_H
#define _MUNMAP_H

#include <stddef.h>
#include <stdint.h>

int munmap(void* addr, size_t len);

#endif
