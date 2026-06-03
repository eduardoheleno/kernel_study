#include "scheduler.h"

#include "memory.h"
#include "tty.h"
#include "pic.h"
#include "timer.h"

task_t *current_task = NULL;
task_t *head_task = NULL;

extern void restore_task_context(cpu_state_t*);

static void task_trampoline()
{
    current_task->entry();

    current_task->status = DEAD;
    scheduler_tick(NULL);
}

static task_t* create_task(void *entry)
{
    task_t *new_task = kmalloc(sizeof(task_t));
    void *task_stack = kmalloc(PAGE_SIZE);

    new_task->context.edi = 0;
    new_task->context.esi = 0;
    new_task->context.ebp = 0;
    new_task->context.ebx = 0;
    new_task->context.edx = 0;
    new_task->context.ecx = 0;
    new_task->context.eax = 0;

    new_task->status = ENQUEUED;
    new_task->stack_size = PAGE_SIZE;
    new_task->entry = entry;
    new_task->stack_base = task_stack;
    new_task->context.cs = 0x08;
    new_task->context.eflags = 0x202;
    new_task->context.esp = (uintptr_t) task_stack + PAGE_SIZE;
    new_task->context.eip = (uint32_t) task_trampoline;

    return new_task;
}

void enqueue_task(void *entry)
{
    task_t *new_task = create_task(entry);

    if (head_task == NULL)
    {
        head_task = new_task;
        new_task->next = head_task;
    }
    else
    {
        task_t *tmp_task = head_task;
        while (tmp_task->next != head_task)
        {
            tmp_task = tmp_task->next;
        }

        tmp_task->next = new_task;
        new_task->next = head_task;
    }
}

void idle_task_loop()
{
    for (;;)
    {
        __asm__ volatile ("hlt");
    }
}

void init_scheduler(void)
{
    task_t *idle_task = create_task(&idle_task_loop);
    idle_task->status = SCHEDULER_IDLE;
    current_task = idle_task;
}

void scheduler_tick(cpu_state_t *state)
{
    if (current_task->status == SCHEDULER_IDLE && head_task == NULL)
    {
        pic_send_eoi(0);
        return;
    }

    if (current_task->status == SCHEDULER_IDLE && head_task != NULL)
    {
        kfree(current_task);
        current_task = head_task;
        current_task->status = RUNNING;
        pic_send_eoi(0);

        restore_task_context(&head_task->context);
        return;
    }

    if (current_task->status == DEAD)
    {
        task_t *tmp_task = head_task;
        while (tmp_task->next != current_task)
        {
            tmp_task = tmp_task->next;
        }

        if (tmp_task == current_task)
        {
            // TODO: insert task stack in some "to free" list
            // kfree(current_task->stack_base);
            kfree(current_task);
            head_task = NULL;

            task_t *idle_task = create_task(&idle_task_loop);
            idle_task->status = SCHEDULER_IDLE;
            current_task = idle_task;

            reset_quantum();
            pic_send_eoi(0);
            restore_task_context(&current_task->context);
            return;
        }

        tmp_task->next = current_task->next;
        if (current_task == head_task)
        {
            head_task = current_task->next;
        }

        task_t *next_task = current_task->next;
        // TODO: insert task stack in some "to free" list
        // kfree(current_task->stack_base);
        kfree(current_task);

        current_task = next_task;
        current_task->status = RUNNING;

        reset_quantum();
        pic_send_eoi(0);
        restore_task_context(&current_task->context);
        return;
    }

    if (current_task->next == current_task)
    {
        pic_send_eoi(0);
        return;
    }

    current_task->context = *state;
    current_task->context.esp = (uint32_t) state + 44;
    current_task->status = INTERRUPTED;

    current_task = current_task->next;
    current_task->status = RUNNING;

    pic_send_eoi(0);
    restore_task_context(&current_task->context);
}
