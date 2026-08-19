all: build/keyboard.o

build/keyboard.o: keyboard.c
	mkdir -p build
	i686-elf-gcc -c keyboard.c -o ./build/keyboard.o -I./include -I../kernel/include -std=gnu99 -ffreestanding -O2 -Wall -Wextra

clean:
	rm -rf /build
