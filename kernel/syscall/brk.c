#include "syscall.h"
#include "scheduler.h"

extern task_t *current_task;

uintptr_t sys_brk(uintptr_t addr)
{
    if (addr == 0) return current_task->program_break;
}
