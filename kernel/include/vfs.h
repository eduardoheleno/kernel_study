#ifndef _KERNEL_VFS_H
#define _KERNEL_VFS_H

#include <stddef.h>
#include <stdint.h>

#define FD_STDIN  0
#define FD_STDOUT 1
#define FD_STDERR 2

struct vnode_ops
{
    size_t (*read)(void *buffer, size_t len);
    void (*write)(const void *buffer, size_t len);
    int (*ioctl)(unsigned long request, void *arg);
    int (*close)(void);
};
typedef struct vnode_ops vnode_ops_t;

struct vnode
{
    const char *name;
    struct vnode *parent;
    struct vnode *children;
    struct vnode *next_sibling;
    struct vnode_ops *ops;
};
typedef struct vnode vnode_t;

struct file
{
    const char *name;
    struct vnode_ops *ops;
    unsigned long flags;
};
typedef struct file file_t;

void init_vfs(void);
file_t* open_file(vnode_t *vnode, uint8_t flags);

#endif
