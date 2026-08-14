#include "stdlib.h"
#include "stdio.h"
#include "munmap.h"

static _Bool is_page_free(mem_block_t* mem_block)
{
    size_t total_free_size = mem_block->parent_page->free_size + mem_block->size + sizeof(mem_block_t) + sizeof(page_block_t);
    page_block_t* parent_page = mem_block->parent_page;
    mem_block_t* it_mem_block = parent_page->free_blocks;
    while (it_mem_block != NULL)
    {
        total_free_size += it_mem_block->size + sizeof(mem_block_t);
        it_mem_block = it_mem_block->next_mem_block;
    }

    if (total_free_size == parent_page->og_size) return 1;
    return 0;
}

static void unmap_free_page(page_block_t* page_block)
{
    if (page_block->next_page_block) page_block->next_page_block->prev_page_block = page_block->prev_page_block;
    if (page_block->prev_page_block) page_block->prev_page_block->next_page_block = page_block->next_page_block;

    if (page_block == mem_pool)
    {
        if (page_block->next_page_block)
        {
            mem_pool = page_block->next_page_block;
        }
        else
        {
            mem_pool = NULL;
        }
    }
    munmap(page_block, page_block->og_size);
}

void free(void* addr)
{
    mem_block_t* mem_block = addr - sizeof(mem_block_t);
    mem_block->is_allocated = 0;

    if (is_page_free(mem_block))
    {
        unmap_free_page(mem_block->parent_page);
        return;
    }

    mem_block_t* it_mem_block = mem_block->parent_page->free_blocks;
    if (it_mem_block == NULL)
    {
        mem_block->parent_page->free_blocks = mem_block;
        return;
    }

    while (it_mem_block->next_mem_block != NULL)
    {
        it_mem_block = it_mem_block->next_mem_block;
    }

    // TODO: only merge blocks if they're adjacent
    if (it_mem_block->is_allocated == 0)
    {
        it_mem_block->size += mem_block->size + sizeof(mem_block_t);
    }
    else
    {
        it_mem_block->next_mem_block = mem_block;
    }
}
