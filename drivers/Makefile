all: keyboard

keyboard: keyboard.c
	i686-elf-gcc -c keyboard.c -o keyboard.o -I./include -I../kernel/include -std=gnu99 -ffreestanding -O2 -Wall -Wextra

clean:
	rm ./*.o
