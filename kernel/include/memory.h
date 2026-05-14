#ifndef _KERNEL_MEMORY_H
#define _KERNEL_MEMORY_H

#include <stdint.h>
#include <stddef.h>

#define KERNEL_BASE 0xC0000000
#define PAGE_MASK 0xFFFFF000
#define PAGE_PRESENT 0x001
#define PAGE_WRITABLE 0x002
#define PAGE_SIZE 4096

#define KERNEL_PAGE_DIRECTORY_INDEX 768

#define BITMAP_USED 1
#define BITMAP_FREE 0

#define PDE_INDEX(addr) (((addr) >> 22) & 0x3FF)
#define PTE_INDEX(addr) (((addr) >> 12) & 0x3FF)

struct kheap_block
{
    size_t size;
    uint8_t free;
    struct kheap_block *next;
    struct kheap_block *prev;
};
typedef struct kheap_block kheap_block_t;

void init_memory_bitmap(unsigned long mbi_addr, unsigned long last_paged_addr, uintptr_t *kernel_page_table_idx);
void* kmalloc(size_t size);
// void* pmm_alloc(uint32_t npages);
// void pmm_free(void *addr, uint32_t npages);


#endif
