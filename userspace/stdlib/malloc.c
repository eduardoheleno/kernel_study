#include "stdlib.h"
#include "mmap.h"
#include "stdio.h"

static page_block_t* mem_pool = NULL;

static size_t align_up(size_t size)
{
    return (size + 7) & ~7;
}

// TODO: improve code reusability
void* malloc(size_t size)
{
    size_t target_size = align_up(size) + sizeof(mem_block_t);
    size_t alloc_size = target_size + sizeof(page_block_t);
    uint32_t total_pages = pages_required(alloc_size);

    if (mem_pool == NULL)
    {
        page_block_t* page_block = mmap(NULL, alloc_size);
        page_block->free_size = (PAGE_SIZE * total_pages) - alloc_size;
        page_block->free_blocks = NULL;
        page_block->next_page_block = NULL;

        mem_block_t* mem_block = (void*)page_block + sizeof(page_block_t);
        mem_block->size = align_up(size);
        mem_block->parent_page = page_block;
        mem_block->next_mem_block = NULL;
        mem_block->is_allocated = 1;

        page_block->next_free_addr = (void*)mem_block + target_size;
        mem_pool = page_block;
        return (void*)mem_block + sizeof(mem_block_t);
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
                while (it_mem_block != NULL)
                {
                    if (it_mem_block->size >= target_size)
                    {
                        // TODO: reuse allocated block and if the size is less then
                        // the target size split it
                    }
                    it_mem_block = it_mem_block->next_mem_block;
                }
            }

            if (it_page_block->free_size >= target_size)
            {
                mem_block_t* new_mem_block = it_page_block->next_free_addr;
                new_mem_block->size = align_up(size);
                new_mem_block->parent_page = it_page_block;
                new_mem_block->is_allocated = 1;
                it_page_block->free_size -= target_size;
                return (void*)new_mem_block + sizeof(mem_block_t);
            }
            prev_it_page_block = it_page_block;
            it_page_block = it_page_block->next_page_block;
        }

        page_block_t* new_page_block = mmap(NULL, alloc_size);
        prev_it_page_block->next_page_block = new_page_block;
        new_page_block->free_size = (PAGE_SIZE * total_pages) - alloc_size;
        new_page_block->free_blocks = NULL;
        new_page_block->next_page_block = NULL;

        mem_block_t* new_mem_block = (void*)new_page_block + sizeof(page_block_t);
        new_mem_block->size = align_up(size);
        new_mem_block->parent_page = new_page_block;
        new_mem_block->next_mem_block = NULL;
        new_mem_block->is_allocated = 1;

        new_page_block->next_free_addr = (void*)new_mem_block + target_size;
        return (void*)new_mem_block + sizeof(mem_block_t);
    }

    return NULL;
}
