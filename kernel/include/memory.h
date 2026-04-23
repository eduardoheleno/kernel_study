#ifndef _KERNEL_MEMORY_H
#define _KERNEL_MEMORY_H

#include <stdint.h>

#define PAGE_SIZE 4096

#define BITMAP_USED 1
#define BITMAP_FREE 0

void init_memory_bitmap(unsigned long mbi_addr, unsigned long last_paged_addr, uintptr_t *boot_page_table_idx);
void* pmm_alloc(uint32_t npages);
void pmm_free(void *addr, uint32_t npages);

#endif
