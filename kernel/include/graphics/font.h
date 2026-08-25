#ifndef _GRAPHICS_FONT_H
#define _GRAPHICS_FONT_H

#include <stdint.h>

#define FONT_WIDTH  8
#define FONT_HEIGHT 8

void put_char(uint32_t x, uint32_t y, uint32_t color, uint8_t char_code);

#endif
