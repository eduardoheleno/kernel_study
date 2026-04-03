#ifndef _KERNEL_IDT_H
#define _KERNEL_IDT_H

#include <stdint.h>

struct idt_entry
{
	uint16_t    isr_low;
	uint16_t    kernel_cs;
	uint8_t     reserved;
	uint8_t     attributes;
	uint16_t    isr_high;
} __attribute__((packed));
typedef struct idt_entry idt_entry_t;

struct idt
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
typedef struct idt idt_t;

void idt_init(void);

#endif
