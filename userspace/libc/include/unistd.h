#ifndef _UNISTD_H
#define _UNISTD_H

#include <stddef.h>

void exit(void);
int read(int fd, void *buf, size_t len);
int write(int fd, const void *buf, size_t len);

#endif

