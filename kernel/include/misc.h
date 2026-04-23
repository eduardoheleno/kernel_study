#ifndef _KERNEL_MISC_H
#define _KERNEL_MISC_H

#include <stdint.h>

static inline uintptr_t align_up_4k(uintptr_t addr)
{
    return (addr + 0xFFF) & ~0xFFF;
}

static inline uintptr_t align_down_4k(uintptr_t addr)
{
    return addr & ~0xFFF;
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
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

#endif
