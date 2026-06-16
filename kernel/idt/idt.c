#include "idt.h"

#include "tty.h"
#include "scheduler.h"

static idt_entry_t idt_entries[256];
static idt_t idt;

extern void *isr_stub_table[];
extern void irq0_stub(void);
extern void irq1_stub(void);
extern void syscall_stub(void);

void exception_handler(uint8_t num)
{
    switch (num)
    {
        case 0:
            terminal_writestring("Divided by 0!\n");
            break;
        case 14:
            terminal_writestring("Page Fault!\n");
            break;
        default:
            terminal_writestring("Exception handler called without error num\n");
            break;
    }

    __asm__ ("cli; hlt");
}

void syscall_handler(cpu_state_t *state)
{
    switch (state->eax)
    {
        case TASK_EXIT:
            task_exit();
            break;
        case 2:
            terminal_writestring("test syscall executed\n");
            break;
    }
}

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
    idt.base = (uintptr_t)&idt_entries[0];
    idt.limit = sizeof(idt_entry_t) * 256 - 1;

    for (uint8_t vector = 0; vector < 32; vector++) 
    {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
    }

    idt_set_descriptor(32, irq0_stub, 0x8E);
    idt_set_descriptor(33, irq1_stub, 0x8E);
    idt_set_descriptor(0x80, syscall_stub, 0xEE);

    __asm__ volatile ("lidt %0" : : "m"(idt));

    terminal_writestring("IDT initialized\n");
}
