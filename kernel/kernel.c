#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gdt.h"
#include "idt.h"
#include "memory.h"
#include "pic.h"
#include "timer.h"
#include "vfs.h"
#include "scheduler.h"
#include "misc.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

// #define MULTIBOOT_BOOTLOADER_MAGIC              0x2BADB002

// TODO: improve code organization
// TODO: check memory alignment on kernel allocation
// TODO: test entire flow of malloc
// TODO: parse psf fonts
void kernel_main(
        uintptr_t *kernel_page_table_idx,
        unsigned long last_paged_addr,
        unsigned long magic,
        unsigned long mbi_addr
     )
{
    init_memory(mbi_addr, last_paged_addr, kernel_page_table_idx);
    gdt_init();
    idt_init();
    pic_init(0x20, 0x28);
    pit_init();
    init_vfs();
    init_scheduler();
    enable_interrupts();

    for (;;) 
    {
        halt();
    }
}
