#ifndef _KERNEL_MEMORY_H
#define _KERNEL_MEMORY_H

#include <stdint.h>
#include <stddef.h>

#define KERNEL_BASE   0xC0000000
#define KHEAP_START   0xC0400000
#define KHEAP_END     0xE0000000

#define PAGE_MASK     0xFFFFF000
#define PAGE_PRESENT  0x001
#define PAGE_WRITABLE 0x002
#define PAGE_SIZE     4096

#define BITMAP_USED 1
#define BITMAP_FREE 0

#define PDE_INDEX(addr) (((addr) >> 22) & 0x3FF)
#define PTE_INDEX(addr) (((addr) >> 12) & 0x3FF)

#define KHEAP_PAGES_NUM (KHEAP_END - KHEAP_START) / PAGE_SIZE

#define SLAB_16   16
#define SLAB_32   32
#define SLAB_64   64
#define SLAB_128  128
#define SLAB_256  256
#define SLAB_512  512
#define SLAB_1024 1024
#define SLAB_2048 2048

#define NUM_CLASSES 8

static uint16_t slab_classes[] = {
    SLAB_16,
    SLAB_32,
    SLAB_64,
    SLAB_128,
    SLAB_256,
    SLAB_512,
    SLAB_1024,
    SLAB_2048
};

// TODO: i think that is necessary to map previous too, cause if some middle slab is freed
// there's no way to attach both sides.
struct slab
{
    struct slab *next;
    struct slab *prev;

    struct slab_cache *cache_owner;

    uint16_t total_count;
    uint16_t free_count;
    void *free_list[];
};
typedef struct slab slab_t;

struct slab_cache
{
    size_t obj_size;

    slab_t *partial;
    slab_t *full;
    // In the current state of the allocator its not possible to have an empty
    // slab, ALL empty slabs are freed.
    // Maybe the allocator should keep some minimum amount of empty slabs
    // to prevent virtual mapping.
    // slab_t *empty;
};
typedef struct slab_cache slab_cache_t;

void init_memory(unsigned long mbi_addr, unsigned long last_paged_addr, uintptr_t *kernel_page_table_idx);
void* kmalloc(size_t size);
void kfree(void *ptr);

#endif
