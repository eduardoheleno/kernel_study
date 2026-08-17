#include "syscall.h"
#include "memory.h"
#include "misc.h"
#include "tty.h"

// ignoring addr param for now
int sys_mmap(void* addr, size_t len)
{
    int first_addr = 0;
    uint32_t mapped_pages = 0;
    uint32_t total_pages = len / PAGE_SIZE;
    if (len % PAGE_SIZE > 0) total_pages++;

    // TODO: optimize this
    // TODO: improve this TERRIBLE code
    uint32_t pte_idx = 0;
    uint32_t pt_idx = 1;
    for (; pte_idx < 1024; pte_idx++)
    {
        for (; pt_idx < 1024; pt_idx++)
        {
            uintptr_t *tmp_pd = map_tmp_page(current_task->cr3);
            uint32_t pt_addr = tmp_pd[pte_idx];
            unmap_tmp_page();
            if (pt_addr == 0x0)
            {
                uintptr_t page_table_phys_addr = pmm_alloc(1);
                uintptr_t *new_page_table = map_tmp_page(page_table_phys_addr);
                for (uint32_t i = 0; i < 1024; i++)
                {
                    new_page_table[i] = 0x0;
                }
                unmap_tmp_page();

                pt_addr = (page_table_phys_addr & PAGE_MASK) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
                uintptr_t *tmp_pd = map_tmp_page(current_task->cr3);
                tmp_pd[pte_idx] = pt_addr;
                unmap_tmp_page();
                uintptr_t page_phys_addr = pmm_alloc(1);
                uintptr_t *new_page = map_tmp_page(page_phys_addr);
                for (uint32_t i = 0; i < 1024; i++)
                {
                    new_page[i] = 0x0;
                }
                unmap_tmp_page();

                uintptr_t *tmp_pt = map_tmp_page(pt_addr);
                tmp_pt[pt_idx] = (page_phys_addr & PAGE_MASK) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
                unmap_tmp_page();

                invlpg((pte_idx << 22) | (pt_idx << 12));
                if (mapped_pages++ == 0) first_addr = (pte_idx << 22) | (pt_idx << 12);
                if (mapped_pages == total_pages) return first_addr;
            }
            else
            {
                uintptr_t *tmp_pt = map_tmp_page(pt_addr);
                if (tmp_pt[pt_idx] == 0x0)
                {
                    unmap_tmp_page();
                    uintptr_t page_phys_addr = pmm_alloc(1);
                    uintptr_t *new_page = map_tmp_page(page_phys_addr);
                    for (uint32_t i = 0; i < 1024; i++)
                    {
                        new_page[i] = 0x0;
                    }
                    unmap_tmp_page();

                    tmp_pt = map_tmp_page(pt_addr);
                    tmp_pt[pt_idx] = (page_phys_addr & PAGE_MASK) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
                    unmap_tmp_page();

                    invlpg((pte_idx << 22) | (pt_idx << 12));
                    if (mapped_pages++ == 0) first_addr = (pte_idx << 22) | (pt_idx << 12);
                    if (mapped_pages == total_pages) return first_addr;
                }
            }
        }
        pt_idx = 0;
    }

    return -1;
}
