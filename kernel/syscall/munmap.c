#include "syscall.h"
#include "memory.h"
#include "misc.h"
#include "tty.h"

int sys_munmap(void* addr, size_t len)
{
    uint32_t current_pages = 0;
    uint32_t total_pages = pages_required(len);
    uint32_t pde_index = PDE_INDEX((int)addr);
    uint32_t pte_index = PTE_INDEX((int)addr);

    for (uint32_t i = pde_index; i < 1024; i++)
    {
        uintptr_t* tmp_pd = map_tmp_page(current_task->cr3);
        uint32_t pt_addr = tmp_pd[i];
        unmap_tmp_page();

        for (uint32_t j = pte_index; j < 1024; j++)
        {
            if (current_pages >= total_pages) break;

            uintptr_t* tmp_pt = map_tmp_page(pt_addr);
            uintptr_t phys_addr = tmp_pt[j];
            tmp_pt[j] = 0x0;
            unmap_tmp_page();
            pmm_free((void*)phys_addr, 1);
            invlpg((i << 22) | (j << 12));
            current_pages++;
        }
    }

    return 0;
}
