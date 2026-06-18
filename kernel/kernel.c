#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "tty.h"
#include "gdt.h"
#include "idt.h"
#include "memory.h"
#include "pic.h"
#include "timer.h"
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

void test1()
{
    volatile uint8_t test = *(volatile uint8_t *)(uintptr_t)0xFFFFFFFF;
    (void)test;

    for(uint8_t i = 0; i < 10; i++)
    {
        terminal_writestring("A");
    }
}

void test2()
{
    for (uint8_t i = 0; i < 10; i++)
    {
        terminal_writestring("B");
    }
}

void test3()
{
    for (uint8_t i = 0; i < 10; i++)
    {
        terminal_writestring("C");
    }
}

// TODO: improve code organization
void kernel_main(
        uintptr_t *kernel_page_table_idx,
        unsigned long last_paged_addr,
        unsigned long magic,
        unsigned long mbi_addr
     )
{
	terminal_initialize();
    gdt_init();
    idt_init();
    pic_init(0x20, 0x28);
    pit_init();
    init_memory(mbi_addr, last_paged_addr, kernel_page_table_idx);
    init_scheduler();
    enable_interrupts();

    enqueue_task(NULL, RING3_TASK);

    // enqueue_task(&test1, RING0_TASK);
    enqueue_task(&test2, RING0_TASK);
    enqueue_task(&test3, RING0_TASK);

    for (;;) 
    {
        halt();
    }
}
