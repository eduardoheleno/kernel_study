ENTRY (_start)

SECTIONS
{
	_kernel_start = .;
    . = 0x00100000;
	/* The kernel will live at 3GB + 1MB in the virtual address space, */
	/* which will be mapped to 1MB in the physical address space. */
	/* Note that we page-align the sections. */

    .multiboot.data : {
        *(.multiboot.data)
    }

    .multiboot.text : {
        *(.multiboot.text)
    }

	. += 0xC0000000;

    _readonly_section = .;
	.text ALIGN (4K) : AT (ADDR (.text) - 0xC0000000)
	{
		*(.text)
	}
	.rodata ALIGN (4K) : AT (ADDR (.rodata) - 0xC0000000)
	{
		*(.rodata)
	}
    /* TODO: _end_readonly_section its not being considered */
    _end_readonly_section = .;

	.data ALIGN (4K) : AT (ADDR (.data) - 0xC0000000)
	{
		*(.data)
	}
	.bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}

	_kernel_end = . - 0xC0000000;
}
