#ifndef _IOCTL_H
#define _IOCTL_H

#define ECHO_OFF (0 << 1)
#define ECHO_ON  (1 << 1)

int ioctl(int fd, unsigned long request, void *arg);

#endif
