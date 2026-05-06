#include "memory.h"

#include "multiboot.h"
#include "misc.h"
#include "tty.h"
#include <stddef.h>

extern char _kernel_start;
extern uintptr_t kernel_page_directory[];

static uintptr_t temp_map_addr;
static uintptr_t kernel_heap_start;

static kheap_block_t *head_block = NULL;
static size_t allocated_pages = 0;

// TODO: current bitmap is using 8 bytes for every page which is pretty non efficient
// The right approach is to use bitwise operations to use a single bit for each page
static uint8_t *bitmap = NULL;
static uint64_t bitmap_size = 0;

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

static void unmap_identity()
{
    asm volatile (
        "movl $0, kernel_page_directory + 0"
    );
}

static uintptr_t* map_temp_page(uintptr_t phys_addr)
{
    uintptr_t pde = kernel_page_directory[PDE_INDEX(temp_map_addr)];
    uint32_t *temp_page_table = (uint32_t*) ((pde & PAGE_MASK) + KERNEL_BASE);
 
    temp_page_table[PTE_INDEX(temp_map_addr)] = (phys_addr & PAGE_MASK) | PAGE_PRESENT | PAGE_WRITABLE;

    invlpg(temp_map_addr);

    return (uintptr_t*) temp_map_addr;
}

static void unmap_temp_page(void)
{
    uintptr_t pde = kernel_page_directory[PDE_INDEX(temp_map_addr)];
    uint32_t *temp_page_table = (uint32_t*) ((pde & PAGE_MASK) + KERNEL_BASE);
 
    temp_page_table[PTE_INDEX(temp_map_addr)] = 0;

    invlpg(temp_map_addr);
}

static uintptr_t pmm_alloc(uint32_t npages)
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

    return ((i - (npages - 1)) * PAGE_SIZE);
}

static void pmm_free(void *addr, uint32_t npages)
{
    // TODO: implement check for non valid addresses
    uint64_t bitmap_idx = (uintptr_t) addr / PAGE_SIZE;
    for (uint64_t i = 0; i < npages; i++) {
        bitmap[bitmap_idx] = BITMAP_FREE;
        bitmap_idx++;
    }
}

void init_memory_bitmap(unsigned long mbi_addr, unsigned long last_paged_addr, uintptr_t *kernel_page_table_idx)
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
        *kernel_page_table_idx = kernel_end_addr | PAGE_PRESENT | PAGE_WRITABLE;

        kernel_end_addr += PAGE_SIZE;
        kernel_page_table_idx++;
    }

    temp_map_addr = kernel_end_addr + KERNEL_BASE;
    kernel_heap_start = temp_map_addr + PAGE_SIZE;

    bitmap = (uint8_t*) align_up_4k(last_paged_addr + KERNEL_BASE);
    for (uint64_t i = 0; i < bitmap_size; i++) {
        bitmap[i] = BITMAP_FREE;
    }

    // terminal_writehex(temp_map_addr);
    // terminal_writestring("\n");
    // terminal_writehex(kernel_end_addr);

    populate_bitmap(mbi, mmap, kernel_end_addr);
    unmap_identity();
    reload_cr3();

    uintptr_t *ptr1 = kmalloc(4096);
    uintptr_t *ptr2 = kmalloc(4096);

    *ptr1 = 10;
    *ptr2 = 20;

    terminal_writeuint(*ptr1);
    terminal_writestring("\n");
    terminal_writeuint(*ptr2);
    //
    // *ptr = 100;
    // terminal_writeuint(*ptr);
    //
    // uintptr_t *ptr2 = kmalloc(10);
    // terminal_writehex((uintptr_t) ptr);

    // terminal_writestring("Page frame allocator initialized!\n");
    // test();
}

uintptr_t* kmalloc(size_t size)
{
    if (head_block == NULL) {
        // TODO: with 1 fixed physical page it could run in trouble if the kernel wants to allocate more than 1 page of size (4096 bytes)
        // TODO: check if all available kernel heap was consumed
        uintptr_t page_phys_addr = pmm_alloc(1);
        uintptr_t *new_page_table = map_temp_page(page_phys_addr);

        for (uint32_t i = 0; i < 1024; i++) {
            new_page_table[i] = 0;
        }

        unmap_temp_page();

        uintptr_t pde = kernel_page_directory[PDE_INDEX(kernel_heap_start)];
        uintptr_t *kernel_page_table = map_temp_page(pde);

        kernel_page_table[PTE_INDEX(kernel_heap_start)] = (page_phys_addr & PAGE_MASK) | PAGE_PRESENT | PAGE_WRITABLE;
        allocated_pages++;

        unmap_temp_page();
        invlpg(kernel_heap_start);

        kheap_block_t *new_block = (kheap_block_t*) kernel_heap_start;
        new_block->free = 0;
        new_block->size = sizeof(kheap_block_t) + size;
        new_block->next = NULL;
        new_block->prev = NULL;

        head_block = new_block;

        return (uintptr_t*) ((uintptr_t) new_block + sizeof(kheap_block_t));
    } else {
        kheap_block_t *tmp = head_block;

        // TODO: search a better approach to track physical addresses mapped to virtual addresses
        for (;;) {
            if (tmp->next == NULL) {
                kheap_block_t *new_block = (kheap_block_t*) ((uintptr_t) tmp + tmp->size);
                size_t allocated_target = ((uintptr_t) new_block - (uintptr_t) head_block) / PAGE_SIZE;
                if (((uintptr_t) new_block - (uintptr_t) head_block) % PAGE_SIZE > 0) allocated_target++;

                if (allocated_pages < allocated_target) {
                    uintptr_t page_phys_addr = pmm_alloc(1);
                    uintptr_t *new_page_table = map_temp_page(page_phys_addr);

                    for (uint32_t i = 0; i < 1024; i++) {
                        new_page_table[i] = 0;
                    }

                    unmap_temp_page();

                    uintptr_t virt_addr = (uintptr_t) head_block + PAGE_SIZE;
                    uintptr_t pde = kernel_page_directory[PDE_INDEX(virt_addr)];
                    uintptr_t *kernel_page_table = map_temp_page(pde);

                    kernel_page_table[PTE_INDEX(virt_addr)] = (page_phys_addr & PAGE_MASK) | PAGE_PRESENT | PAGE_WRITABLE;
                    allocated_pages++;

                    unmap_temp_page();
                    invlpg(virt_addr);
                }

                new_block->free = 0;
                new_block->size = sizeof(kheap_block_t) + size;
                new_block->next = NULL;
                new_block->prev = tmp;

                tmp->next = new_block;
                return (uintptr_t*) ((uintptr_t) new_block + sizeof(kheap_block_t));
            }

            tmp = tmp->next;
        }
    }
}
