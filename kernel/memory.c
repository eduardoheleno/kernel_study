#include "memory.h"

#include "misc.h"
#include "tty.h"
#include <stddef.h>

extern char __kernel_end[];

static multiboot_memory_map_t* fetch_highest_mem_block(multiboot_info_t *mbi, multiboot_memory_map_t *mmap)
{
    multiboot_memory_map_t *ptr = NULL;
    while ((uintptr_t) mmap < mbi->mmap_addr + mbi->mmap_length) {
        if (ptr == NULL) ptr = (multiboot_memory_map_t*) mmap;
        if (
            mmap->type == MULTIBOOT_MEMORY_AVAILABLE &&
            (uintptr_t) mmap->len > ptr->len
        ) {
            ptr = (multiboot_memory_map_t*) mmap;
        }
        mmap = (multiboot_memory_map_t*) ((uintptr_t) mmap + mmap->size + sizeof(uint32_t));
    }

    return ptr;
}

static uintptr_t insert_metadata_offset(multiboot_memory_map_t *mem_block)
{
    uint64_t kernel_end = align_up_4k((uintptr_t) __kernel_end);
    for (uint64_t ptr = mem_block->addr; ptr < mem_block->addr + mem_block->len; ptr += PAGE_SIZE) {
        if (ptr > (uint64_t) align_up_4k(kernel_end + sizeof(memory_metadata_t))) {
            return ptr;
        }
    }

    return (uintptr_t) NULL;
}

void init_bitmap_memory(unsigned long mbi_addr)
{
    multiboot_info_t *mbi = (multiboot_info_t*) mbi_addr;
    multiboot_memory_map_t *mmap = (multiboot_memory_map_t*) mbi->mmap_addr;

    multiboot_memory_map_t *highest_mem_block = fetch_highest_mem_block(mbi, mmap);
    uintptr_t offset_addr = insert_metadata_offset(highest_mem_block);

    memory_metadata_t *memory_metadata = (memory_metadata_t*) align_up_4k((uintptr_t) __kernel_end);
    memory_metadata->base_addr = offset_addr;

    for (uint64_t i = 0; i < TOTAL_PAGES; i++) {
        memory_metadata->pages[i].status = PAGE_STATUS_AVAILABLE;
        memory_metadata->pages[i].pages = 0;
    }

    terminal_writestring("Physical memory initialized!\n");
}

void kmfree(void *ptr)
{
    memory_metadata_t *memory_metadata = (memory_metadata_t*) align_up_4k((uintptr_t) __kernel_end);
    uint32_t page_index = ((uintptr_t) ptr - memory_metadata->base_addr) / PAGE_SIZE;

    memory_metadata->pages[page_index].pages = 0;
    memory_metadata->pages[page_index].status = PAGE_STATUS_AVAILABLE;
}

void* kmalloc(uint32_t size)
{
    if (size == 0) return NULL;

    memory_metadata_t *memory_metadata = (memory_metadata_t*) align_up_4k((uintptr_t) __kernel_end);
    uint32_t page_index = 0;
    uint32_t pages = (size / PAGE_SIZE) - 1;
    if (size % PAGE_SIZE > 0) pages++;

    for (; page_index < TOTAL_PAGES; page_index++) {
        if (memory_metadata->pages[page_index].status != PAGE_STATUS_AVAILABLE) {
            page_index += memory_metadata->pages[page_index].pages;
            continue;
        }

        uint32_t continuous_pages = 0;
        for (uint32_t j = page_index + 1; j < TOTAL_PAGES; j++) {
            if (continuous_pages == pages) break;
            if (memory_metadata->pages[j].status == PAGE_STATUS_ALLOCATED) break;
            if (memory_metadata->pages[j].status == PAGE_STATUS_AVAILABLE) continuous_pages++;
        }

        if (continuous_pages == pages) break;
    }

    memory_metadata->pages[page_index].status = PAGE_STATUS_ALLOCATED;
    memory_metadata->pages[page_index].pages = pages;

    return (void*) (memory_metadata->base_addr + (page_index * PAGE_SIZE));
}
