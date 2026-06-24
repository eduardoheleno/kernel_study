#ifndef _DRIVER_KEYBOARD_H
#define _DRIVER_KEYBOARD_H

#include <stdint.h>
#include <stddef.h>

#define KEYBOARD_BUFFER_SIZE 4096

enum KYBRD_CTRL_STATS_MASK {
	KYBRD_CTRL_STATS_MASK_OUT_BUF	=	1,          //00000001
	KYBRD_CTRL_STATS_MASK_IN_BUF	=	2,          //00000010
	KYBRD_CTRL_STATS_MASK_SYSTEM	=	4,          //00000100
	KYBRD_CTRL_STATS_MASK_CMD_DATA	=	8,          //00001000
	KYBRD_CTRL_STATS_MASK_LOCKED	=	0x10,		//00010000
	KYBRD_CTRL_STATS_MASK_AUX_BUF	=	0x20,		//00100000
	KYBRD_CTRL_STATS_MASK_TIMEOUT	=	0x40,		//01000000
	KYBRD_CTRL_STATS_MASK_PARITY	=	0x80		//10000000
};

size_t read_keyboard_buffer(char *buffer, size_t len);
int keyboard_buffer_has_line(void);

#endif
