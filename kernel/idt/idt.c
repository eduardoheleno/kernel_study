#include "idt.h"

#include "tty.h"

static idt_entry_t idt_entries[256];
static idt_t idt;

extern void *isr_stub_table[];
extern void irq1_stub(void);

static void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags)
{
    idt_entry_t *descriptor = &idt_entries[vector];

    descriptor->isr_low = (uint32_t)isr & 0xFFFF;
    descriptor->kernel_cs = 0x08;
    descriptor->attributes = flags;
    descriptor->isr_high = (uint32_t)isr >> 16;
    descriptor->reserved = 0;
}

void idt_init(void)
{
    idt.base = (uintptr_t) &idt_entries[0];
    idt.limit = sizeof(idt_entry_t) * 256 - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
    }

    idt_set_descriptor(33, irq1_stub, 0x8E);

    __asm__ volatile ("lidt %0" : : "m"(idt));

    terminal_writestring("IDT initialized!\n");
}
