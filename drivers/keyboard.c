#include "keyboard.h"

#include "pic.h"
#include "misc.h"
#include "tty.h"
#include "scheduler.h"

static const char *scancodes[] = 
{
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
    NULL, NULL, "\n", NULL,
    "a", "s", "d", "f", "g", "h", "j", "k", "l",
    NULL, NULL, NULL, NULL, NULL,
    "z", "x", "c", "v", "b", "n", "m", ",",
    NULL, NULL, NULL, NULL, NULL, " "
};
char k_buffer[KEYBOARD_BUFFER_SIZE];
size_t buffer_head = 0;
size_t buffer_tail = 0;

// static uint8_t kybrd_ctrl_read_status()
// {
//     return inb(0x64);
// }
//
// static void kybrd_ctrl_send_cmd(uint8_t cmd)
// {
//     while (1) 
//     {
//         if ((kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_IN_BUF) == 0) break;
//     }
//
//     outb(0x64, cmd);
// }

void test()
{
    static uint8_t test = 0;
    for (uint8_t i = 0; i < 10; i++)
    {
        terminal_writeuint(test);
    }

    test++;
}

void read_keyboard_buffer(char *buffer, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (buffer_head == buffer_tail) break;
        if (buffer_head >= KEYBOARD_BUFFER_SIZE) buffer_head = 0;
        buffer[i] = k_buffer[buffer_head++];
    }
}

int keyboard_buffer_has_line(void)
{
    for (size_t i = buffer_head; i < buffer_tail; i++)
    {
        if (k_buffer[i] == '\n') return 1;
    }

    return -1;
}

void keyboard_interrupt_handler(void)
{
    uint8_t scancode = inb(0x60);

    if ((scancode & 0x80) == 0) 
    {
        if (*scancodes[scancode] == 't')
        {
            enqueue_task(NULL, RING3_TASK);
            pic_send_eoi(1);
            return;
        }

        if (buffer_tail < KEYBOARD_BUFFER_SIZE)
        {
            k_buffer[buffer_tail++] = *scancodes[scancode];
        }
        if (buffer_tail >= KEYBOARD_BUFFER_SIZE && buffer_head == buffer_tail)
        {
            buffer_tail = 0;
            k_buffer[buffer_tail++] = *scancodes[scancode];
        }
        if (*scancodes[scancode] == '\n') wake_stdin_task();
    }

    pic_send_eoi(1);
}
