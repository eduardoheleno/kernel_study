.PHONY: all crt unistd stdio string libc program clean

all: crt unistd stdio string ioctl libc program

crt:
	i686-elf-gcc -ffreestanding -nostdlib -fno-pie -m32 -I ./libc/include/ -c ./crt/crt0.c -o crt0.o

unistd:
	i686-elf-gcc -ffreestanding -nostdlib -fno-pie -m32 -I ./libc/include/ -c ./libc/unistd/exit.c -o exit.o
	i686-elf-gcc -ffreestanding -nostdlib -fno-pie -m32 -I ./libc/include/ -c ./libc/unistd/read.c -o read.o
	i686-elf-gcc -ffreestanding -nostdlib -fno-pie -m32 -I ./libc/include/ -c ./libc/unistd/write.c -o write.o

stdio:
	i686-elf-gcc -ffreestanding -nostdlib -fno-pie -m32 -I ./libc/include/ -c ./libc/stdio/printf.c -o printf.o
	i686-elf-gcc -ffreestanding -nostdlib -fno-pie -m32 -I ./libc/include/ -c ./libc/stdio/fgets.c -o fgets.o

string:
	i686-elf-gcc -ffreestanding -nostdlib -fno-pie -m32 -I ./libc/include/ -c ./libc/string.c -o string.o

ioctl:
	i686-elf-gcc -ffreestanding -nostdlib -fno-pie -m32 -I ./libc/include/ -c ./libc/ioctl.c -o ioctl.o

libc:
	i686-elf-ar rcs libc.a exit.o read.o write.o printf.o fgets.o string.o ioctl.o

program:
	i686-elf-gcc -ffreestanding -nostdlib -fno-pie -m32 -I ./libc/include/ -c ./programs/hello.c -o hello.o
	i686-elf-ld -m elf_i386 -Ttext=0x00001000 -e _start crt0.o hello.o libc.a -o hello.elf
	i686-elf-objcopy -O binary hello.elf hello.bin

clean:
	rm -f *.o *.a *.elf *.bin
