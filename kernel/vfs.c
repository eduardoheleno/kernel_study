#include "vfs.h"
#include "memory.h"
#include "tty.h"

vnode_t *global_vfs_root = NULL;
vnode_t *global_tty = NULL;

static vnode_t* create_vnode(const char *name)
{
    vnode_t *vnode = kmalloc(sizeof(vnode_t));
    vnode->name = name;
    vnode->parent = NULL;
    vnode->children = NULL;
    vnode->next_sibling = NULL;
    vnode->ops = NULL;

    return vnode;
}

static void mount_file_on_dir(vnode_t *file, vnode_t *dir)
{
    file->parent = dir;
    if (dir->children == NULL)
    {
        dir->children = file;
    }
    else
    {
        vnode_t *tmp = dir->children;
        while (tmp->next_sibling)
        {
            tmp = tmp->next_sibling;
        }
        tmp->next_sibling = file;
    }
}

void init_vfs(void)
{
    global_vfs_root = create_vnode("/");
    vnode_t *dev_dir = create_vnode("dev");
    global_tty = create_vnode("tty");

    global_tty->ops = tty_ops();

    mount_file_on_dir(global_tty, dev_dir);
    mount_file_on_dir(dev_dir, global_vfs_root);
}

file_t* open_file(vnode_t *vnode, uint8_t flags)
{
    file_t *file = kmalloc(sizeof(file_t));
    file->name = vnode->name;
    file->ops = vnode->ops;
    file->flags = flags;

    return file;
}
