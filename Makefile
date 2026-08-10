.PHONY: all kernel drivers userspace

all: drivers userspace kernel

kernel:
	$(MAKE) -C kernel

drivers:
	$(MAKE) -C drivers

userspace:
	$(MAKE) -C userspace

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C drivers clean
	$(MAKE) -C userspace clean
