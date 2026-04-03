#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

struct gdt_entry
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));
typedef struct gdt_entry gdt_entry_t;

struct gdt
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));
typedef struct gdt gdt_t;

void gdt_init();

#endif
