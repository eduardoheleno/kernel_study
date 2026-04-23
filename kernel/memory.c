#include "memory.h"

#include "misc.h"
#include "tty.h"
#include <stddef.h>

static multiboot_memory_map_t* fetch_highest_block(multiboot_info_t *mbi, multiboot_memory_map_t *mmap)
{
    for (;;) {
        if ((uintptr_t) mmap + mmap->size + sizeof(uint32_t) >= mbi->mmap_addr + mbi->mmap_length) break;
        mmap = (multiboot_memory_map_t*) ((uintptr_t) mmap + mmap->size + sizeof(uint32_t));
    }

    return mmap;
}

void init_bitmap_memory(unsigned long mbi_addr, unsigned long last_paged_addr, uintptr_t *boot_page_table_idx)
{
    multiboot_info_t *mbi = (multiboot_info_t*) mbi_addr;
    multiboot_memory_map_t *mmap = (multiboot_memory_map_t*) mbi->mmap_addr;

    multiboot_memory_map_t *highest_block = fetch_highest_block(mbi, mmap);
    uint64_t highest_addr = highest_block->addr + highest_block->len;

    uint64_t total_bitmap_idxs = highest_addr / PAGE_SIZE;
    uintptr_t kernel_end_addr = align_up_4k(last_paged_addr);

    uint64_t bitmap_size = total_bitmap_idxs * sizeof(uint8_t);
    uint64_t total_bitmap_pages = bitmap_size / PAGE_SIZE;

    for (uint64_t i = 0; i < total_bitmap_pages; i++) {
        *boot_page_table_idx = kernel_end_addr | 0x003;

        kernel_end_addr += PAGE_SIZE;
        boot_page_table_idx++;
    }

    invlpg(0);

    uint8_t *bitmap = (uint8_t*) align_up_4k(last_paged_addr);
    for (uint64_t i = 0; i < total_bitmap_idxs; i++) {
        bitmap[i] = 0;
    }

    terminal_writestring("mapping");
    // uintptr_t bitmap_addr = align_up_4k(kernel_end_addr);
    // uint8_t bitmap[total_pages] = bitmap_addr;
    // terminal_writehex(bitmap_addr);

    // terminal_writeuint(total_pages);
    // terminal_writeuint(highest_addr);
    // uint64_t bitmap_size = (last_avaiable_block->addr + last_avaiable_block->len) / PAGE_SIZE;
    // terminal_writehex(last_avaiable_address->addr);
    // terminal_writestring(" - ");
    // terminal_writeuint(last_avaiable_address->len);
    // las
    // mmap

    // multiboot_memory_map_t *highest_mem_block = fetch_highest_mem_block(mbi, mmap);
    // terminal_writeuint(highest_mem_block->type);
    // highest_mem_block->
    // uintptr_t offset_addr = insert_metadata_offset(highest_mem_block);

    // memory_metadata_t *memory_metadata = (memory_metadata_t*) align_up_4k((uintptr_t) __kernel_end);
    // memory_metadata->base_addr = offset_addr;
    //
    // for (uint64_t i = 0; i < TOTAL_PAGES; i++) {
    //     memory_metadata->pages[i].status = PAGE_STATUS_AVAILABLE;
    //     memory_metadata->pages[i].pages = 0;
    // }

    // terminal_writestring("Physical memory initialized!\n");
}

// void kmfree(void *ptr)
// {
//     memory_metadata_t *memory_metadata = (memory_metadata_t*) align_up_4k((uintptr_t) __kernel_end);
//     uint32_t page_index = ((uintptr_t) ptr - memory_metadata->base_addr) / PAGE_SIZE;
//
//     memory_metadata->pages[page_index].pages = 0;
//     memory_metadata->pages[page_index].status = PAGE_STATUS_AVAILABLE;
// }
//
// void* kmalloc(uint32_t size)
// {
//     if (size == 0) return NULL;
//
//     memory_metadata_t *memory_metadata = (memory_metadata_t*) align_up_4k((uintptr_t) __kernel_end);
//     uint32_t page_index = 0;
//     uint32_t pages = (size / PAGE_SIZE) - 1;
//     if (size % PAGE_SIZE > 0) pages++;
//
//     for (; page_index < TOTAL_PAGES; page_index++) {
//         if (memory_metadata->pages[page_index].status != PAGE_STATUS_AVAILABLE) {
//             page_index += memory_metadata->pages[page_index].pages;
//             continue;
//         }
//
//         uint32_t continuous_pages = 0;
//         for (uint32_t j = page_index + 1; j < TOTAL_PAGES; j++) {
//             if (continuous_pages == pages) break;
//             if (memory_metadata->pages[j].status == PAGE_STATUS_ALLOCATED) break;
//             if (memory_metadata->pages[j].status == PAGE_STATUS_AVAILABLE) continuous_pages++;
//         }
//
//         if (continuous_pages == pages) break;
//     }
//
//     memory_metadata->pages[page_index].status = PAGE_STATUS_ALLOCATED;
//     memory_metadata->pages[page_index].pages = pages;
//
//     return (void*) (memory_metadata->base_addr + (page_index * PAGE_SIZE));
// }
