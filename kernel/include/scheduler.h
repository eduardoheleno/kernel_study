#ifndef _KERNEL_SCHEDULER_H
#define _KERNEL_SCHEDULER_H

#include <stdint.h>
#include <stddef.h>

struct cpu_state
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
};
typedef struct cpu_state cpu_state_t;

typedef void (task_entry_t)(void);

struct task
{
    cpu_state_t context;
    void *stack_base;
    size_t stack_size;
    task_entry_t *entry;
    
    struct task *next;
};
typedef struct task task_t;

void enqueue_task();
void scheduler_tick(cpu_state_t *state);

#endif
