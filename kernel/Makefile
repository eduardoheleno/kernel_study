# TODO: organize ALL makefiles

all: boot_asm gdt_asm idt_asm scheduler_asm kernel link run

boot_asm: boot.s
	i686-elf-as boot.s -o boot.o

gdt_asm: ./gdt/gdt.s
	i686-elf-as ./gdt/gdt.s -o gdt_asm.o

idt_asm: ./idt/idt.s
	i686-elf-as ./idt/idt.s -o idt_asm.o

scheduler_asm: ./scheduler/scheduler.s
	i686-elf-as ./scheduler/scheduler.s -o scheduler_asm.o

kernel: kernel.c tty.c memory.c pic.c timer.c ./gdt/gdt.c ./idt/idt.c ./scheduler/scheduler.c ./vfs.c ./syscall/mmap.c ./syscall/read.c ./syscall/write.c ./syscall/ioctl.c
	i686-elf-gcc -c kernel.c tty.c pic.c timer.c memory.c ./gdt/gdt.c ./idt/idt.c ./scheduler/scheduler.c ./vfs.c ./syscall/mmap.c ./syscall/read.c ./syscall/write.c ./syscall/ioctl.c -I./include -I../drivers/include -std=gnu99 -ffreestanding -O2 -Wall -Wextra

link: linker.ld
	i686-elf-gcc -T linker.ld -o myos -ffreestanding -O2 -nostdlib boot.o gdt_asm.o gdt.o idt_asm.o idt.o scheduler_asm.o scheduler.o tty.o memory.o pic.o timer.o vfs.o mmap.o read.o write.o ioctl.o ../drivers/keyboard.o kernel.o

run:
	qemu-system-i386 -display cocoa,zoom-to-fit=on -m 3G -kernel myos -initrd ../userspace/hello.bin

clean:
	rm ./*.o ./myos
