#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "tty.h"
#include "gdt.h"
#include "idt.h"
#include "memory.h"
#include "pic.h"
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

void kernel_main(
        uintptr_t *kernel_page_table_idx,
        unsigned long last_paged_addr,
        unsigned long magic,
        unsigned long mbi_addr
     ) 
{
	terminal_initialize();

    init_memory(mbi_addr, last_paged_addr, kernel_page_table_idx);

    // uint32_t *test1 = pmm_alloc(1);
    // terminal_writehex((uint32_t) test1);
    // terminal_writestring("\n");

    //
    // uint32_t *test2 = pmm_alloc(2);
    // terminal_writehex((uint32_t) test2);
    // terminal_writestring("\n");
    //
    // uint32_t *test3 = pmm_alloc(3);
    // terminal_writehex((uint32_t) test3);
    // terminal_writestring("\n");

    // gdt_init();
    // idt_init();
    // pic_init(0x20, 0x28);
    // __asm__ volatile ("sti");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
