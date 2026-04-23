#ifndef _KERNEL_MEMORY_H
#define _KERNEL_MEMORY_H

#include <stdint.h>

#define PAGE_SIZE 4096

void init_bitmap_memory(unsigned long mbi_addr, unsigned long last_paged_addr, uintptr_t *boot_page_table_idx);
void* kmalloc(uint32_t size);
void kmfree(void *ptr);

#endif
