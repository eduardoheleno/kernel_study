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

#endif
