#include "stdlib.h"
#include "mmap.h"
#include "stdio.h"

page_block_t* mem_pool = NULL;

static size_t align_up(size_t size)
{
    return (size + 7) & ~7;
}

static page_block_t* alloc_page_block(size_t size)
{
    uint32_t total_pages = pages_required(size);
    page_block_t *new_page_block = mmap(NULL, size);
    new_page_block->og_size = (PAGE_SIZE * total_pages);
    new_page_block->free_size = (PAGE_SIZE * total_pages) - sizeof(page_block_t);
    new_page_block->free_blocks = NULL;
    new_page_block->next_page_block = NULL;
    new_page_block->prev_page_block = NULL;
    new_page_block->next_free_addr = (void*)new_page_block + sizeof(page_block_t);
    return new_page_block;
}

static mem_block_t* alloc_mem_block(size_t size, page_block_t* page_parent)
{
    mem_block_t* new_mem_block = page_parent->next_free_addr;
    page_parent->free_size -= size;
    page_parent->next_free_addr += size;
    new_mem_block->size = size - sizeof(mem_block_t);
    new_mem_block->parent_page = page_parent;
    new_mem_block->next_mem_block = NULL;
    new_mem_block->is_allocated = 1;
    return new_mem_block;
}

void* malloc(size_t size)
{
    size_t aligned_size = align_up(size);
    size_t target_size = aligned_size + sizeof(mem_block_t);
    size_t alloc_size = target_size + sizeof(page_block_t);
    if (mem_pool == NULL)
    {
        page_block_t* new_page_block = alloc_page_block(alloc_size);
        mem_block_t* new_mem_block = alloc_mem_block(target_size, new_page_block);
        mem_pool = new_page_block;
        return (void*)new_mem_block + sizeof(mem_block_t);
    }
    else
    {
        page_block_t* it_page_block = mem_pool;
        page_block_t* prev_it_page_block = NULL;
        while (it_page_block != NULL)
        {
            if (it_page_block->free_blocks)
            {
                mem_block_t* it_mem_block = it_page_block->free_blocks;
                mem_block_t* prev_it_mem_block = NULL;
                while (it_mem_block != NULL)
                {
                    if (it_mem_block->size > aligned_size)
                    {
                        mem_block_t* splitted_mem_block = (mem_block_t*)((uint8_t*)it_mem_block + target_size);
                        splitted_mem_block->size = it_mem_block->size - target_size;
                        splitted_mem_block->is_allocated = 0;
                        splitted_mem_block->parent_page = it_mem_block->parent_page;

                        it_mem_block->size = aligned_size;
                        it_mem_block->is_allocated = 1;
                        splitted_mem_block->next_mem_block = it_mem_block->next_mem_block;
                        if (prev_it_mem_block == NULL)
                        {
                            it_mem_block->parent_page->free_blocks = splitted_mem_block;
                        }
                        else
                        {
                            prev_it_mem_block->next_mem_block = splitted_mem_block;
                        }

                        it_mem_block->next_mem_block = NULL;

                        // == Test ==
                        mem_block_t* it_mem_block_test = it_mem_block->parent_page->free_blocks;
                        while (it_mem_block_test != NULL)
                        {
                            printf("size: %i\n", it_mem_block_test->size);
                            printf("is_allocated: %i\n", it_mem_block_test->is_allocated);
                            it_mem_block_test = it_mem_block_test->next_mem_block;
                        }
                        return (void*)it_mem_block + sizeof(mem_block_t);
                    }

                    if (it_mem_block->size == aligned_size)
                    {
                        it_mem_block->is_allocated = 1;
                        if (prev_it_mem_block == NULL)
                        {
                            it_mem_block->parent_page->free_blocks = it_mem_block->next_mem_block;
                        }
                        else
                        {
                            prev_it_mem_block->next_mem_block = it_mem_block->next_mem_block;
                        }

                        return (void*)it_mem_block + sizeof(mem_block_t);
                    }
                    prev_it_mem_block = it_mem_block;
                    it_mem_block = it_mem_block->next_mem_block;
                }
            }

            if (it_page_block->free_size >= target_size)
            {
                mem_block_t* new_mem_block = alloc_mem_block(target_size, it_page_block);
                return (void*)new_mem_block + sizeof(mem_block_t);
            }
            prev_it_page_block = it_page_block;
            it_page_block = it_page_block->next_page_block;
        }

        page_block_t* new_page_block = alloc_page_block(alloc_size);
        new_page_block->prev_page_block = prev_it_page_block;
        prev_it_page_block->next_page_block = new_page_block;
        mem_block_t* new_mem_block = alloc_mem_block(target_size, new_page_block);
        return (void*)new_mem_block + sizeof(mem_block_t);
    }

    return NULL;
}
