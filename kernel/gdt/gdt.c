#include "gdt.h"

#include "tty.h"
#include <stdint.h>

extern void gdt_flush(void);

static gdt_entry_t gdt_entries[3];
gdt_t gdt;

static void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = ((limit >> 16) & 0x0F);

    gdt_entries[num].granularity |= (gran & 0xF0);
    gdt_entries[num].access = access;
}

void gdt_init()
{
    gdt.limit = (sizeof(gdt_entry_t) * 3) - 1;
    gdt.base = (uintptr_t) gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdt_flush();

    terminal_writestring("GDT initialized!\n");
}
