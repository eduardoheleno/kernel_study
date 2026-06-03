.globl restore_task_context
restore_task_context:
    movl 4(%esp), %eax

    movl 12(%eax), %esp
    pushl 40(%eax)
    pushl 36(%eax)
    pushl 32(%eax)

    movl 0(%eax), %edi
    movl 4(%eax), %esi
    movl 8(%eax), %ebp
    movl 16(%eax), %ebx
    movl 20(%eax), %edx
    movl 24(%eax), %ecx
    movl 28(%eax), %eax

    iret
