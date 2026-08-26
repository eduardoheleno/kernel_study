#ifndef _GRAPHICS_FONT_H
#define _GRAPHICS_FONT_H

#include <stdint.h>
#include <stddef.h>

struct psf2 {
    unsigned char magic[4];
    unsigned int version;
    unsigned int headersize;    /* offset of bitmaps in file */
    unsigned int flags;
    unsigned int length;        /* number of glyphs */
    unsigned int charsize;      /* number of bytes for each character */
    unsigned int height, width; /* max dimensions of glyphs */
};

void put_char(uint32_t x, uint32_t y, uint32_t color, uint8_t char_code);
void load_psf2_font(void);
void test(void);

#endif
