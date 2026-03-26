all: asm kernel link run

asm: boot.s
	i686-elf-as boot.s -o boot.o

kernel: kernel.c
	i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

link: linker.ld
	i686-elf-gcc -T linker.ld -o myos -ffreestanding -O2 -nostdlib boot.o kernel.o

run:
	qemu-system-i386 -m 3G -kernel myos

clean:
	rm ./*.o ./myos
