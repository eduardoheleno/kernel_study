#include "filesystem/fs.h"
#include "filesystem/disk.h"
#include "memory.h"
#include "tty.h"
#include "misc.h"

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

static void init_ibmap(void)
{
    uint8_t bitmap_area[4096];
    kmemset(bitmap_area, 0, sizeof(bitmap_area));
    disk_write(9, 8, bitmap_area);
}

static void init_dbmap(void)
{
    uint8_t bitmap_area[4096];
    kmemset(bitmap_area, 0, sizeof(bitmap_area));
    disk_write(17, 8, bitmap_area);
}

static void init_iblock(void)
{
    uint8_t* iblock_area = kmalloc(20480);
    kmemset(iblock_area, 0, 20480);
    disk_write(25, 40, iblock_area);
    kfree(iblock_area);
}

void init_fs(void)
{
    // TODO: check possible error
    init_disk();
    init_ibmap();
    init_dbmap();
    init_iblock();

    inode_t root;
    root.type = DIR_TYPE;
    root.size = 68;
    root.blocks = 1;

    uint8_t inode_buffer[512];
    kmemset(inode_buffer, 0, sizeof(inode_buffer));
    kmemcpy(inode_buffer, &root, sizeof(root));

    disk_write(25, 1, inode_buffer);

    uint8_t inode_buffer2[512];
    disk_read(25, 1, (uint16_t*)inode_buffer2);

    inode_t root2;
    kmemcpy(&root2, inode_buffer2, sizeof(inode_t));
    debug_int(root2.size);
    
    // disk_read(25, 1, (uint16_t*)inode_buffer);
    //
    // inode_t root = *(inode_t*)inode_buffer;
    // debug_int(root.type);

    // entries[0].name = ".";

    // uint8_t magic_buffer[512];
    // disk_read(0, 1, (uint16_t*)magic_buffer);
    //
    // terminal_writestring("\n");
    // terminal_writehex(*(uint32_t*)magic_buffer);
    // terminal_writestring("\n");

    // uint8_t magic_buffer[512];
    // kmemset(magic_buffer, 0, sizeof(magic_buffer));
    //
    // uint16_t magic = 0x776;
    // kmemcpy(magic_buffer, &magic, sizeof(magic));
    //
    // disk_write(0, 1, magic_buffer);
}

void init_vfs(multiboot_info_t* mbi)
{
    multiboot_module_t* mbm = (multiboot_module_t*)mbi->mods_addr;
    tar_header* th = (tar_header*)mbm->mod_start;
    // th = (tar_header*)((uint32_t)th + 512);

    debug_write(th->file_path);
    // debug_hex32((uint32_t)th);
    // debug_write("\n");
    // debug_hex32(mbm->mod_start + 512);

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
