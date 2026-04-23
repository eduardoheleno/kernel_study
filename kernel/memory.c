#include "memory.h"

#include "multiboot.h"
#include "misc.h"
#include "tty.h"
#include <stddef.h>

extern char _kernel_start;
uint8_t *bitmap = NULL;
uint64_t bitmap_size = 0;

static multiboot_memory_map_t* fetch_highest_block(multiboot_info_t *mbi, multiboot_memory_map_t *mmap)
{
    for (;;) {
        if ((uintptr_t) mmap + mmap->size + sizeof(uint32_t) >= mbi->mmap_addr + mbi->mmap_length) break;
        mmap = (multiboot_memory_map_t*) ((uintptr_t) mmap + mmap->size + sizeof(uint32_t));
    }

    return mmap;
}

static void set_used_bitmaps(uint64_t start_index, uint64_t total_used_pages)
{
    for (uint64_t i = 0; i < total_used_pages; i++) {
        bitmap[start_index] = BITMAP_USED;
        start_index++;
    }
}

static void populate_bitmap(multiboot_info_t *mbi, multiboot_memory_map_t *mmap, uintptr_t kernel_end_addr)
{
    for (;;) {
        // set used based on GRUB
        if (mmap->type != MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t start_index = mmap->addr / PAGE_SIZE;
            uint64_t range_size = (mmap->addr + mmap->len) - mmap->addr;
            uint64_t total_used_pages = range_size / PAGE_SIZE;
            if (range_size % PAGE_SIZE > 0) total_used_pages++;
            set_used_bitmaps(start_index, total_used_pages);
        }

        multiboot_memory_map_t *next_mmap = (multiboot_memory_map_t*) ((uintptr_t) mmap + mmap->size + sizeof(uint32_t));
        if ((uintptr_t) next_mmap >= mbi->mmap_addr + mbi->mmap_length) break;

        if (mmap->addr + mmap->len < next_mmap->addr) {
            // set used based on gap
            uint64_t start_index = (mmap->addr + mmap->len) / PAGE_SIZE;
            uint64_t range_size = next_mmap->addr - (mmap->addr + mmap->len);
            uint64_t total_used_pages = range_size / PAGE_SIZE;
            if (range_size % PAGE_SIZE > 0) total_used_pages++;
            set_used_bitmaps(start_index, total_used_pages);
        }
        
        mmap = (multiboot_memory_map_t*) ((uintptr_t) mmap + mmap->size + sizeof(uint32_t));
    }

    // set used based on kernel and bitmap size
    uint64_t start_index = (uint64_t) _kernel_start / PAGE_SIZE;
    uint64_t range_size = kernel_end_addr - (uint64_t) _kernel_start;
    uint64_t total_used_pages = range_size / PAGE_SIZE;
    if (range_size % PAGE_SIZE > 0) total_used_pages++;
    set_used_bitmaps(start_index, total_used_pages);
}

void init_memory_bitmap(unsigned long mbi_addr, unsigned long last_paged_addr, uintptr_t *boot_page_table_idx)
{
    multiboot_info_t *mbi = (multiboot_info_t*) mbi_addr;
    multiboot_memory_map_t *mmap = (multiboot_memory_map_t*) mbi->mmap_addr;

    multiboot_memory_map_t *highest_block = fetch_highest_block(mbi, mmap);
    uint64_t highest_addr = highest_block->addr + highest_block->len;

    bitmap_size = highest_addr / PAGE_SIZE;
    uintptr_t kernel_end_addr = align_up_4k(last_paged_addr);

    uint64_t bitmap_len = bitmap_size * sizeof(uint8_t);
    uint64_t total_bitmap_pages = bitmap_len / PAGE_SIZE;

    for (uint64_t i = 0; i < total_bitmap_pages; i++) {
        *boot_page_table_idx = kernel_end_addr | 0x003;

        kernel_end_addr += PAGE_SIZE;
        boot_page_table_idx++;
    }

    invlpg(0);

    bitmap = (uint8_t*) align_up_4k(last_paged_addr);
    for (uint64_t i = 0; i < bitmap_size; i++) {
        bitmap[i] = BITMAP_FREE;
    }

    populate_bitmap(mbi, mmap, kernel_end_addr);

    terminal_writestring("Page frame allocator initialized!\n");
}

void* pmm_alloc(uint32_t npages)
{
    // TODO: implement check for full physical memory usage
    uint64_t i = 0;
    uint32_t page_accumulator = 0;
    for (; i < bitmap_size; i++) {
        if (bitmap[i] != BITMAP_FREE) {
            page_accumulator = 0;
        } else {
            page_accumulator++;
            if (page_accumulator == npages) break;
        }
    }

    for (uint64_t j = i - (npages - 1); j <= i; j++) {
        bitmap[j] = BITMAP_USED;
    }

    return (void*) ((i - (npages - 1)) * PAGE_SIZE);
}

void pmm_free(void *addr, uint32_t npages)
{
    //TODO: implement check for non valid addresses
    uint64_t bitmap_idx = (uintptr_t) addr / PAGE_SIZE;
    for (uint64_t i = 0; i < npages; i++) {
        bitmap[bitmap_idx] = BITMAP_FREE;
        bitmap_idx++;
    }
}
