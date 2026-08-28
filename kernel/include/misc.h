#ifndef _KERNEL_MISC_H
#define _KERNEL_MISC_H

#include <stdint.h>
#include "memory.h"

static inline uintptr_t align_up_4k(uintptr_t addr)
{
    return (addr + 0xFFF) & ~0xFFF;
}

static inline uintptr_t align_down_4k(uintptr_t addr)
{
    return addr & ~0xFFF;
}

static inline void disable_interrupts(void)
{
    __asm__ volatile ("cli");
}

static inline void enable_interrupts(void)
{
    __asm__ volatile ("sti");
}

static inline void halt(void)
{
    __asm__ volatile ("hlt");
}

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ("inb %w1, %b0"
            : "=a"(ret)
            : "Nd"(port)
            : "memory");

    return ret;
}

static inline void io_wait(void)
{
    outb(0x80, 0);
}

static inline void invlpg(unsigned long addr) 
{
    __asm__ volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

static inline void reload_cr3(void) 
{
    __asm__ volatile (
        "mov %%cr3, %%eax\n\t"
        "mov %%eax, %%cr3"
        :
        :
        : "eax", "memory"
    );
}

static inline void load_cr3(uintptr_t pd_phys_addr)
{
    __asm__ volatile (
        "mov %0, %%cr3"
        :
        : "r"(pd_phys_addr)
        : "memory"
    );
}

static inline void unmap_identity()
{
    asm volatile (
        "movl $0, kernel_page_directory + 0"
    );
}

static inline uint32_t pages_required(uint32_t len)
{
    return (len + PAGE_SIZE - 1) / PAGE_SIZE;
}

static inline void debug_putc(char c)
{
    outb(0xE9, c);
}

static inline void debug_write(const char *str)
{
    while (*str)
    {
        debug_putc(*str++);
    }
}

static inline void debug_int(int32_t value)
{
    char buffer[10];
    uint32_t magnitude;
    uint32_t length = 0;

    if (value < 0)
    {
        debug_putc('-');
        magnitude = (uint32_t)(-(value + 1)) + 1;
    }
    else
    {
        magnitude = (uint32_t)value;
    }

    do
    {
        buffer[length++] = '0' + (magnitude % 10);
        magnitude /= 10;
    }
    while (magnitude != 0);

    while (length != 0)
    {
        debug_putc(buffer[--length]);
    }
}

static inline void debug_hex32(uint32_t value)
{
    static const char hex[] = "0123456789ABCDEF";

    debug_write("0x");

    for (int shift = 28; shift >= 0; shift -= 4)
    {
        debug_putc(hex[(value >> shift) & 0xF]);
    }
}

static inline void debug_hex64(uint64_t value)
{
    static const char hex[] = "0123456789ABCDEF";

    debug_write("0x");

    for (int shift = 60; shift >= 0; shift -= 4)
    {
        debug_putc(hex[(value >> shift) & 0xF]);
    }
}

#endif
