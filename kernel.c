#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

extern char __kernel_end[];
extern void gdt_flush(void);

/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

typedef unsigned char           multiboot_uint8_t;
typedef unsigned short          multiboot_uint16_t;
typedef unsigned int            multiboot_uint32_t;
typedef unsigned long long      multiboot_uint64_t;

struct multiboot_aout_symbol_table
{
  multiboot_uint32_t tabsize;
  multiboot_uint32_t strsize;
  multiboot_uint32_t addr;
  multiboot_uint32_t reserved;
};
typedef struct multiboot_aout_symbol_table multiboot_aout_symbol_table_t;

struct multiboot_elf_section_header_table
{
  multiboot_uint32_t num;
  multiboot_uint32_t size;
  multiboot_uint32_t addr;
  multiboot_uint32_t shndx;
};
typedef struct multiboot_elf_section_header_table multiboot_elf_section_header_table_t;

struct multiboot_info
{
  /* Multiboot info version number */
  multiboot_uint32_t flags;

  /* Available memory from BIOS */
  multiboot_uint32_t mem_lower;
  multiboot_uint32_t mem_upper;

  /* "root" partition */
  multiboot_uint32_t boot_device;

  /* Kernel command line */
  multiboot_uint32_t cmdline;

  /* Boot-Module list */
  multiboot_uint32_t mods_count;
  multiboot_uint32_t mods_addr;

  union
  {
    multiboot_aout_symbol_table_t aout_sym;
    multiboot_elf_section_header_table_t elf_sec;
  } u;

  /* Memory Mapping buffer */
  multiboot_uint32_t mmap_length;
  multiboot_uint32_t mmap_addr;

  /* Drive Info buffer */
  multiboot_uint32_t drives_length;
  multiboot_uint32_t drives_addr;

  /* ROM configuration table */
  multiboot_uint32_t config_table;

  /* Boot Loader Name */
  multiboot_uint32_t boot_loader_name;

  /* APM table */
  multiboot_uint32_t apm_table;

  /* Video */
  multiboot_uint32_t vbe_control_info;
  multiboot_uint32_t vbe_mode_info;
  multiboot_uint16_t vbe_mode;
  multiboot_uint16_t vbe_interface_seg;
  multiboot_uint16_t vbe_interface_off;
  multiboot_uint16_t vbe_interface_len;

  multiboot_uint64_t framebuffer_addr;
  multiboot_uint32_t framebuffer_pitch;
  multiboot_uint32_t framebuffer_width;
  multiboot_uint32_t framebuffer_height;
  multiboot_uint8_t framebuffer_bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT     2
  multiboot_uint8_t framebuffer_type;
  union
  {
    struct
    {
      multiboot_uint32_t framebuffer_palette_addr;
      multiboot_uint16_t framebuffer_palette_num_colors;
    };
    struct
    {
      multiboot_uint8_t framebuffer_red_field_position;
      multiboot_uint8_t framebuffer_red_mask_size;
      multiboot_uint8_t framebuffer_green_field_position;
      multiboot_uint8_t framebuffer_green_mask_size;
      multiboot_uint8_t framebuffer_blue_field_position;
      multiboot_uint8_t framebuffer_blue_mask_size;
    };
  };
};
typedef struct multiboot_info multiboot_info_t;

struct multiboot_mmap_entry
{
  multiboot_uint32_t size;
  multiboot_uint64_t addr;
  multiboot_uint64_t len;
#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5
  multiboot_uint32_t type;
} __attribute__((packed));
typedef struct multiboot_mmap_entry multiboot_memory_map_t;

#define TOTAL_PAGES 1024

struct page_metadata
{
#define PAGE_STATUS_AVAILABLE 1
#define PAGE_STATUS_ALLOCATED 2
    uint8_t status;
    uint64_t pages;
};
typedef struct page_metadata page_metadata_t;

struct memory_metadata
{
    uintptr_t base_addr;
    page_metadata_t pages[TOTAL_PAGES];
};
typedef struct memory_metadata memory_metadata_t;

#define PAGE_SIZE 4096

multiboot_info_t *mbi;

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

gdt_entry_t gdt_entries[3];
gdt_t gdt;

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000 
#define MULTIBOOT_BOOTLOADER_MAGIC              0x2BADB002

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;

void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) 
{
    if (c == '\n') {
        terminal_row++;
        terminal_column = 0;
        return;
    }

	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}

void terminal_write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) 
{
	terminal_write(data, strlen(data));
}

void terminal_writeuint(uint32_t value) 
{
    char buffer[11];
    int i = 0;

    if (value == 0) {
        terminal_putchar('0');
        return;
    }

    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0) {
        terminal_putchar(buffer[--i]);
    }
}

void terminal_writehex(uint32_t value)
{
    char hex_digits[] = "0123456789ABCDEF";

    terminal_putchar('0');
    terminal_putchar('x');

    for (int i = 28; i >= 0; i -= 4)
    {
        uint8_t digit = (value >> i) & 0xF;
        terminal_putchar(hex_digits[digit]);
    }
}

uintptr_t align_up_4k(uintptr_t addr)
{
    return (addr + 0xFFF) & ~0xFFF;
}

uintptr_t align_down_4k(uintptr_t addr)
{
    return addr & ~0xFFF;
}

multiboot_memory_map_t* fetch_highest_mem_block(multiboot_memory_map_t *mmap)
{
    multiboot_memory_map_t *ptr = NULL;
    while ((uintptr_t) mmap < mbi->mmap_addr + mbi->mmap_length) {
        if (ptr == NULL) ptr = (multiboot_memory_map_t*) mmap;
        if (
            mmap->type == MULTIBOOT_MEMORY_AVAILABLE &&
            (uintptr_t) mmap->len > ptr->len
        ) {
            ptr = (multiboot_memory_map_t*) mmap;
        }
        mmap = (multiboot_memory_map_t*) ((uintptr_t) mmap + mmap->size + sizeof(uint32_t));
    }

    return ptr;
}

uintptr_t insert_metadata_offset(multiboot_memory_map_t *mem_block)
{
    uint64_t kernel_end = align_up_4k((uintptr_t) __kernel_end);
    for (uint64_t ptr = mem_block->addr; ptr < mem_block->addr + mem_block->len; ptr += PAGE_SIZE) {
        if (ptr > (uint64_t) align_up_4k(kernel_end + sizeof(memory_metadata_t))) {
            return ptr;
        }
    }

    return (uintptr_t) NULL;
}

void init_bitmap_memory(multiboot_memory_map_t *mmap)
{
    multiboot_memory_map_t *highest_mem_block = fetch_highest_mem_block(mmap);
    uintptr_t offset_addr = insert_metadata_offset(highest_mem_block);

    memory_metadata_t *memory_metadata = (memory_metadata_t*) align_up_4k((uintptr_t) __kernel_end);
    memory_metadata->base_addr = offset_addr;

    for (uint64_t i = 0; i < TOTAL_PAGES; i++) {
        memory_metadata->pages[i].status = PAGE_STATUS_AVAILABLE;
        memory_metadata->pages[i].pages = 0;
    }

    terminal_writestring("Physical memory initiated!\n");
}

void* kmalloc(uint32_t size)
{
    if (size == 0) return NULL;

    memory_metadata_t *memory_metadata = (memory_metadata_t*) align_up_4k((uintptr_t) __kernel_end);
    uint32_t page_index = 0;
    uint32_t pages = (size / PAGE_SIZE) - 1;
    if (size % PAGE_SIZE > 0) pages++;

    for (; page_index < TOTAL_PAGES; page_index++) {
        if (memory_metadata->pages[page_index].status != PAGE_STATUS_AVAILABLE) {
            page_index += memory_metadata->pages[page_index].pages;
            continue;
        }

        uint32_t continuous_pages = 0;
        for (uint32_t j = page_index + 1; j < TOTAL_PAGES; j++) {
            if (continuous_pages == pages) break;
            if (memory_metadata->pages[j].status == PAGE_STATUS_ALLOCATED) break;
            if (memory_metadata->pages[j].status == PAGE_STATUS_AVAILABLE) continuous_pages++;
        }

        if (continuous_pages == pages) break;
    }

    memory_metadata->pages[page_index].status = PAGE_STATUS_ALLOCATED;
    memory_metadata->pages[page_index].pages = pages;

    return (void*) (memory_metadata->base_addr + (page_index * PAGE_SIZE));
}

void kmfree(void *ptr)
{
    memory_metadata_t *memory_metadata = (memory_metadata_t*) align_up_4k((uintptr_t) __kernel_end);
    uint32_t page_index = ((uintptr_t) ptr - memory_metadata->base_addr) / PAGE_SIZE;

    memory_metadata->pages[page_index].pages = 0;
    memory_metadata->pages[page_index].status = PAGE_STATUS_AVAILABLE;
}

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = ((limit >> 16) & 0x0F);

    gdt_entries[num].granularity |= (gran & 0xF0);
    gdt_entries[num].access = access;
}

void gdt_install()
{
    gdt.limit = (sizeof(gdt_entry_t) * 3) - 1;
    gdt.base = (uintptr_t) gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdt_flush();

    terminal_writestring("GDT initiated!\n");
}

void kernel_main(unsigned long magic, unsigned long mbi_addr) 
{
    mbi = (multiboot_info_t*) mbi_addr;
    multiboot_memory_map_t *mmap = (multiboot_memory_map_t*) mbi->mmap_addr;

	terminal_initialize();

    init_bitmap_memory(mmap);
    gdt_install();
}
