#ifndef _DRIVER_KEYBOARD_H
#define _DRIVER_KEYBOARD_H

#include <stdint.h>

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

uint8_t kybrd_ctrl_read_status();
void kybrd_ctrl_send_cmd(uint8_t cmd);
void keyboard_interrupt_handler(void);

#endif
