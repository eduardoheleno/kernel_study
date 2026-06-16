.globl gdt_flush
.globl tss_flush

.extern gdt

gdt_flush:
    lgdt gdt
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ljmp $0x08, $flush2

tss_flush:
    movw $0x28, %ax
    ltr %ax
    ret

flush2:
    ret
