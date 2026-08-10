#include "stdlib.h"
#include "stdio.h"

void free(void* addr)
{
    mem_block_t* mem_block = addr - sizeof(mem_block_t);
    mem_block->is_allocated = 0;

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

    if (it_mem_block->is_allocated == 0)
    {
        it_mem_block->size += mem_block->size;
    }
    else
    {
        it_mem_block->next_mem_block = mem_block;
    }

    // // == Test ==
    // mem_block_t* it_mem_block_test = mem_block->parent_page->free_blocks;
    // while (it_mem_block_test != NULL)
    // {
    //     printf("size: %i\n", it_mem_block_test->size);
    //     printf("is_allocated: %i\n", it_mem_block_test->is_allocated);
    //     it_mem_block_test = it_mem_block_test->next_mem_block;
    // }
}
