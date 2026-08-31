# alphaOS
A Unix-like 32-bit OS for studying purposes.

## Features
- GDT
- IDT
- Physical memory management
- Paging
- Scheduler
- Basic userspace
- PS/2 keyboard driver

## GUI
The OS doesn't have a graphical interface yet but the font rendering already uses the framebuffer with a psf2 font parser to draw pixels on the screen.

---

_**NOTE**: This OS doesn't boot on real machines cause it relies on legacy hardware (like Intel 8253/8254 chip), only runnable for emulators._
