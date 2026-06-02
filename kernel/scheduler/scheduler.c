#include "scheduler.h"

#include "memory.h"
#include "tty.h"

task_t *current_task = NULL;

void test()
{
    for (;;)
    {
        static uint64_t test = 0;
        terminal_writeuint(test++);
        terminal_writestring("\n");
    }
}

void enqueue_task()
{
    terminal_writestring("scheduled!\n");
    task_t *new_task = kmalloc(sizeof(task_t));
    void *task_stack = kmalloc(PAGE_SIZE);

    new_task->context.edi = 0;
    new_task->context.esi = 0;
    new_task->context.ebp = 0;
    new_task->context.ebx = 0;
    new_task->context.edx = 0;
    new_task->context.ecx = 0;
    new_task->context.eax = 0;

    new_task->context.cs = 0x08;
    new_task->context.eflags = 0x202;
    new_task->context.esp = (uintptr_t) task_stack + PAGE_SIZE;
    new_task->context.eip = (uint32_t) &test;

    if (current_task == NULL) {
        current_task = new_task;
    } else {
        task_t *tmp_task = current_task;
        while (tmp_task->next != NULL) 
        {
            tmp_task = tmp_task->next;
        }

        tmp_task->next = new_task;
    }
}

void scheduler_tick(cpu_state_t *state)
{

}
