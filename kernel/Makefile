all: boot_asm gdt_asm idt_asm kernel link run

boot_asm: boot.s
	i686-elf-as boot.s -o boot.o

gdt_asm: ./gdt/gdt.s
	i686-elf-as ./gdt/gdt.s -o gdt_asm.o

idt_asm: ./idt/idt.s
	i686-elf-as ./idt/idt.s -o idt_asm.o

kernel: kernel.c tty.c memory.c ./gdt/gdt.c ./idt/idt.c
	i686-elf-gcc -c kernel.c tty.c memory.c ./gdt/gdt.c ./idt/idt.c -I./include -std=gnu99 -ffreestanding -O2 -Wall -Wextra

link: linker.ld
	i686-elf-gcc -T linker.ld -o myos -ffreestanding -O2 -nostdlib boot.o gdt_asm.o gdt.o idt_asm.o idt.o tty.o memory.o kernel.o

run:
	qemu-system-i386 -m 3G -kernel myos

clean:
	rm ./*.o ./myos
