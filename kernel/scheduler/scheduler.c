#include "scheduler.h"

#include "memory.h"
#include "pic.h"
#include "timer.h"
#include "misc.h"
#include "tty.h"

static task_t *idle_task = NULL;
static task_t *reaper_task = NULL;
static task_t *current_task = NULL;
static task_t *dead_queue = NULL;

static uint64_t task_total = 0;
static uint64_t next_pid = 0;

extern void restore_task_context(cpu_state_t*);

static void wake_idle_task(void)
{
    idle_task->status = TASK_SCHEDULER_IDLE;
}

static void wake_reaper_task(void)
{
    reaper_task->status = TASK_READY;
}

static void sleep_reaper_task(void)
{
    reaper_task->status = TASK_SLEEP;
}

static task_t* next_task(void)
{
    task_t *tmp_task = current_task->next;
    while (tmp_task->status != TASK_READY && tmp_task->status != TASK_SCHEDULER_IDLE)
    {
        tmp_task = tmp_task->next;
    }

    return tmp_task;
}

static void push_dead_queue(task_t *task)
{
    task->next = NULL;
    if (dead_queue == NULL)
    {
        dead_queue = task;
    }
    else
    {
        task_t *tmp_task = dead_queue;
        while (tmp_task->next != NULL)
        {
            tmp_task = tmp_task->next;
        }

        tmp_task->next = task;
    }
}

static task_t* pop_dead_queue(void)
{
    if (dead_queue == NULL) return NULL;
    task_t *popped_task = dead_queue;
    dead_queue = dead_queue->next;

    return popped_task;
}

static void task_exit(void)
{
    disable_interrupts();

    task_total--;
    task_t *tmp_task = current_task;
    while (tmp_task->next != current_task)
    {
        tmp_task = tmp_task->next;
    }

    tmp_task->next = current_task->next;

    task_t *ntask = next_task();
    push_dead_queue(current_task);
    wake_reaper_task();
    current_task = ntask;
    if (task_total > 0)
    {
        current_task->status = TASK_RUNNING;
    }

    enable_interrupts();

    reset_quantum();
    restore_task_context(&current_task->context);
}

static void task_trampoline(void)
{
    current_task->entry();
    task_exit();
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

    new_task->pid = next_pid++;
    new_task->status = TASK_READY;
    new_task->stack_size = PAGE_SIZE;
    new_task->entry = entry;
    new_task->stack_base = task_stack;
    new_task->context.cs = 0x08;
    new_task->context.eflags = 0x202;
    new_task->context.esp = (uintptr_t) task_stack + PAGE_SIZE;
    new_task->context.eip = (uint32_t) task_trampoline;

    new_task->next = NULL;

    return new_task;
}

void enqueue_task(void *entry)
{
    task_t *new_task = create_task(entry);
    task_t * tmp_task = current_task;
    while (tmp_task->next != current_task)
    {
        tmp_task = tmp_task->next;
    }

    tmp_task->next = new_task;
    new_task->next = current_task;
    task_total++;
}

void idle_task_loop(void)
{
    for (;;)
    {
        halt();
    }
}

void reaper_task_loop(void)
{
    for (;;)
    {
        while (dead_queue != NULL)
        {
            task_t *dead_task = pop_dead_queue();
            kfree(dead_task->stack_base);
            kfree(dead_task);
        }

        sleep_reaper_task();
    }
}

void init_scheduler(void)
{
    idle_task = create_task(&idle_task_loop);
    idle_task->status = TASK_SCHEDULER_IDLE;

    reaper_task = create_task(&reaper_task_loop);
    reaper_task->status = TASK_SLEEP;

    idle_task->next = reaper_task;
    reaper_task->next = idle_task;

    current_task = idle_task;
}

void scheduler_tick(cpu_state_t *state)
{
    if (task_total == 0)
    {
        wake_idle_task();
    }

    task_t *ntask = next_task();
    if (ntask == current_task)
    {
        pic_send_eoi(0);
        return;
    }

    if (current_task->status == TASK_SCHEDULER_IDLE)
    {
        current_task->status = TASK_SLEEP;
    }
    else
    {
        current_task->status = TASK_READY;
        current_task->context = *state;
        current_task->context.esp = (uint32_t) state + 44;
    }

    current_task = ntask;
    current_task->status = TASK_RUNNING;
    pic_send_eoi(0);
    restore_task_context(&current_task->context);
}
