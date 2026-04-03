#ifndef _KERNEL_MEMORY_H
#define _KERNEL_MEMORY_H

#include <stdint.h>
#include "multiboot.h"

#define TOTAL_PAGES 1024
#define PAGE_SIZE 4096

struct page_metadata
{
#define PAGE_STATUS_AVAILABLE 1
#define PAGE_STATUS_ALLOCATED 2
    uint8_t status;
    uint64_t pages;
};
typedef struct page_metadata page_metadata_t;

struct memory_metadata
{
    uintptr_t base_addr;
    page_metadata_t pages[TOTAL_PAGES];
};
typedef struct memory_metadata memory_metadata_t;

void init_bitmap_memory(unsigned long mbi_addr);
void* kmalloc(uint32_t size);
void kmfree(void *ptr);

#endif
