all: boot_asm gdt_asm idt_asm kernel link run

boot_asm: boot.s
	i686-elf-as boot.s -o boot.o

gdt_asm: gdt.s
	i686-elf-as gdt.s -o gdt.o

idt_asm: idt.s
	i686-elf-as idt.s -o idt.o

kernel: kernel.c
	i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

link: linker.ld
	i686-elf-gcc -T linker.ld -o myos -ffreestanding -O2 -nostdlib boot.o gdt.o idt.o kernel.o

run:
	qemu-system-i386 -m 3G -kernel myos

clean:
	rm ./*.o ./myos
