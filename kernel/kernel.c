#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "misc.h"
#include "tty.h"
#include "gdt.h"
#include "idt.h"
#include "memory.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

void terminal_writestring(const char* data);

void exception_handler() 
{
    terminal_writestring("exception_handler\n");
    __asm__ ("cli; hlt");
}

#define MULTIBOOT_BOOTLOADER_MAGIC              0x2BADB002

#define PIC1            0x20
#define PIC2            0xA0
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1 + 1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2 + 1)

#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */

#define CASCADE_IRQ 2
void pic_init(int offset1, int offset2)
{
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, offset1);
    io_wait();
    outb(PIC2_DATA, offset2);
    io_wait();

    outb(PIC1_DATA, 1 << CASCADE_IRQ);
    io_wait();
    outb(PIC2_DATA, 2);
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, 0xFD);
    outb(PIC2_DATA, 0xFF);

    terminal_writestring("PIC initialized!\n");
}

enum KYBRD_CTRL_STATS_MASK {
	KYBRD_CTRL_STATS_MASK_OUT_BUF	=	1,          //00000001
	KYBRD_CTRL_STATS_MASK_IN_BUF	=	2,          //00000010
	KYBRD_CTRL_STATS_MASK_SYSTEM	=	4,          //00000100
	KYBRD_CTRL_STATS_MASK_CMD_DATA	=	8,          //00001000
	KYBRD_CTRL_STATS_MASK_LOCKED	=	0x10,		//00010000
	KYBRD_CTRL_STATS_MASK_AUX_BUF	=	0x20,		//00100000
	KYBRD_CTRL_STATS_MASK_TIMEOUT	=	0x40,		//01000000
	KYBRD_CTRL_STATS_MASK_PARITY	=	0x80		//10000000
};

uint8_t kybrd_ctrl_read_status()
{
    return inb(0x64);
}

void kybrd_ctrl_send_cmd(uint8_t cmd)
{
    while (1) {
        if ((kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_IN_BUF) == 0) break;
    }

    outb(0x64, cmd);
}

static void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8) {
        outb(PIC2_COMMAND, 0x20);
    }

    outb(PIC1_COMMAND, 0x20);
}

const char *scancodes[] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
    NULL, NULL, "\n", NULL,
    "a", "s", "d", "f", "g", "h", "j", "k", "l",
    NULL, NULL, NULL, NULL, NULL,
    "z", "x", "c", "v", "b", "n", "m", ",",
    NULL, NULL, NULL, NULL, NULL, " "
};

void keyboard_interrupt_handler(void)
{
    uint8_t scancode = inb(0x60);

    if ((scancode & 0x80) == 0) {
        terminal_writestring(scancodes[scancode]);
    }

    pic_send_eoi(1);
}

void kernel_main(unsigned long magic, unsigned long mbi_addr) 
{
	terminal_initialize();

    init_bitmap_memory(mbi_addr);
    gdt_init();
    idt_init();
    pic_init(0x20, 0x28);
    __asm__ volatile ("sti");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
