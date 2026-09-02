#ifndef _KERNEL_VFS_H
#define _KERNEL_VFS_H

#include <stddef.h>
#include <stdint.h>
#include "multiboot.h"

#define FD_STDIN  0
#define FD_STDOUT 1
#define FD_STDERR 2

#define FILE_TYPE 1
#define DIR_TYPE  2

struct inode
{
    uint8_t type;
    uint16_t size;
    uint32_t blocks;
    uint32_t block[15];
};
typedef struct inode inode_t;

struct dir_entry
{
    uint32_t inode_number;
    char name[28];
};

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

struct tar_header
{
	char file_path[100];
	char file_mode[8];
	char owner_user_id[8];
	char owner_group_id[8];
	char file_size[12];
	char file_mtime[12];
	char header_checksum[8];
	char file_type;
	char link_path[100];

	char padding[255];
};
typedef struct tar_header tar_header;

void init_fs(void);
void init_vfs(multiboot_info_t* mbi);
file_t* open_file(vnode_t *vnode, uint8_t flags);

#endif
