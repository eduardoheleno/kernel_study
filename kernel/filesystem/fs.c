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
    disk_write(IBLOCK_OFFSET, 40, iblock_area);
    kfree(iblock_area);
}

static int alloc_inode_num(void)
{
    uint8_t inode_bmap[4096];
    kmemset(inode_bmap, 0, sizeof(inode_bmap));
    disk_read(9, 8, (uint16_t*)inode_bmap);

    for (uint16_t i = 0; i < 4096; i++)
    {
        for (uint16_t j = 0; j < 8; j++)
        {
            uint8_t bit = (inode_bmap[i] >> j) & 1;
            if (bit == 0)
            {
                inode_bmap[i] |= (1 << j);
                disk_write(9, 8, inode_bmap);
                return i * 8 + j;
            }
        }
    }

    return -1;
}

static void write_inode_data(inode_t* inode, uint8_t* data, size_t size)
{
    uint8_t data_bmap[4096];
    kmemset(data_bmap, 0, sizeof(data_bmap));
    disk_read(17, 8, (uint16_t*)data_bmap);

    uint8_t* data_buffer = kmalloc(512 * inode->sectors);
    kmemset(data_buffer, 0, 512 * inode->sectors);
    kmemcpy(data_buffer, data, size);
    uint32_t sectors_count = 0;
    for (uint16_t i = 0; i < 4096; i++)
    {
        for (uint16_t j = 0; j < 8; j++)
        {
            if (sectors_count >= inode->sectors) break;

            uint8_t bit = (data_bmap[i] >> j) & 1;
            if (bit == 0)
            {
                data_bmap[i] |= (1 << j);
                uint32_t sector_idx = i * 8 + j;
                inode->sector[sectors_count] = sector_idx;
                disk_write(DATA_OFFSET + sector_idx, 1, &data_buffer[sectors_count++ * 512]);
            }
        }
    }

    disk_write(17, 8, data_bmap);
    kfree(data_buffer);
}

static uint8_t* read_inode_data(inode_t inode)
{
    uint8_t* data_buffer = kmalloc(512 * inode.sectors);
    for (uint32_t i = 0; i < inode.sectors; i++)
    {
        disk_read(DATA_OFFSET + inode.sector[i], 1, (uint16_t*)&data_buffer[i * 512]);
    }

    return data_buffer;
}

static void write_inode(inode_t inode, uint32_t inode_num)
{
    uint32_t inode_sector = inode_num / 8;
    uint32_t inode_offset = inode_num % 8;
    uint8_t sector_buffer[512];
    disk_read(IBLOCK_OFFSET + inode_sector, 1, (uint16_t*)sector_buffer);

    kmemcpy(&sector_buffer[inode_offset * sizeof(inode_t)], &inode, sizeof(inode_t));
    disk_write(IBLOCK_OFFSET + inode_sector, 1, sector_buffer);
}

static void read_inode(uint32_t inode_num, inode_t* inode_buffer)
{
    uint32_t inode_sector = inode_num / 8;
    uint32_t inode_offset = inode_num % 8;
    uint8_t sector_buffer[512];
    disk_read(IBLOCK_OFFSET + inode_sector, 1, (uint16_t*)sector_buffer);

    kmemcpy(inode_buffer, &sector_buffer[inode_offset * sizeof(inode_t)], sizeof(inode_t));
}



void init_fs(multiboot_info_t* mbi)
{
    // TODO: check possible error
    init_disk();
    init_ibmap();
    init_dbmap();
    init_iblock();

    multiboot_module_t* mbm = (multiboot_module_t*)mbi->mods_addr;
    tar_header* th = (tar_header*)mbm->mod_start;

    uint64_t file_size = tar_parse_octal(th->file_size, sizeof(th->file_size));
    uint8_t* file_data = (uint8_t*)th + sizeof(*th);
    // file_data[0] -> file_data[file_size - 1]
    
    th = (tar_header *)(
        file_data + ((file_size + 511) & ~(uint64_t)511)
    );

    // th = (tar_header *)(
    //     (uint8_t *)th +
    //     512 +
    //     ((tar_parse_octal(th->file_size, sizeof(th->file_size)) + 511) & ~511)
    // );
    // th = (tar_header *)(
    //     (uint8_t *)th +
    //     512 +
    //     ((tar_parse_octal(th->file_size, sizeof(th->file_size)) + 511) & ~511)
    // );
    // th = (tar_header *)(
    //     (uint8_t *)th +
    //     512 +
    //     ((tar_parse_octal(th->file_size, sizeof(th->file_size)) + 511) & ~511)
    // );
    // th = (tar_header *)(
    //     (uint8_t *)th +
    //     512 +
    //     ((tar_parse_octal(th->file_size, sizeof(th->file_size)) + 511) & ~511)
    // );
    // th = (tar_header *)(
    //     (uint8_t *)th +
    //     512 +
    //     ((tar_parse_octal(th->file_size, sizeof(th->file_size)) + 511) & ~511)
    // );
    // debug_write(th->file_path);

    // int iroot_num = alloc_inode_num();
    // struct dir_entry root_entries[2];
    // root_entries[0].inode_number = iroot_num;
    // root_entries[1].inode_number = iroot_num;
    // kmemcpy(root_entries[0].name, ".", 1);
    // kmemcpy(root_entries[1].name, "..", 2);
    //
    // inode_t root_inode;
    // root_inode.type = DIR_TYPE;
    // root_inode.size = sizeof(root_entries);
    // root_inode.sectors = (sizeof(root_entries) + 511) / 512;
    //
    // write_inode_data(&root_inode, (uint8_t*)root_entries, sizeof(root_entries));
    // write_inode(root_inode, iroot_num);
    //
    // read_inode_data(root_inode);
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
