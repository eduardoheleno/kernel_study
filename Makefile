.PHONY: all kernel drivers userspace

all: drivers userspace kernel run

kernel:
	$(MAKE) -C kernel

drivers:
	$(MAKE) -C drivers

userspace:
	$(MAKE) -C userspace

run:
	qemu-system-i386 -display cocoa,zoom-to-fit=on -m 3G -debugcon stdio -global isa-debugcon.iobase=0xe9 -drive file=disk.img,format=raw,if=ide,index=0 -cdrom myos.iso

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C drivers clean
	$(MAKE) -C userspace clean
