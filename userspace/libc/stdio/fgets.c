#include "stdio.h"
#include "unistd.h"
#include "ioctl.h"

void fgets(char *buffer, size_t len, int fd)
{
    unsigned long flags;
    flags |= ECHO_FLAG;
    ioctl(fd, SET_FLAG_REQUEST, (void*)flags);
    read(fd, buffer, len);
    ioctl(fd, CLEAR_FLAG_REQUEST, (void*)flags);
}
