#include "graphics/font.h"
#include "graphics/framebuffer.h"

extern char _binary_zap_ext_light24_psf_start[];
extern char _binary_zap_ext_light24_psf_end[];
extern char _binary_zap_ext_light24_psf_size[];

static struct psf2* psf2_data = NULL;

void load_psf2_font(void)
{
    psf2_data = (struct psf2*)_binary_zap_ext_light24_psf_start;
}

void put_char(uint32_t x, uint32_t y, uint32_t color, uint8_t char_code)
{
    uint8_t* glyphs = _binary_zap_ext_light24_psf_start + psf2_data->headersize;
    uint8_t* glyph = glyphs + (char_code * psf2_data->charsize);
    uint32_t bytes_per_line = (psf2_data->width + 7) / 8;
    uint32_t x_origin = x;
    for (uint8_t i = 0; i < psf2_data->height; i++)
    {
        for (uint8_t j = 0; j < psf2_data->width; j++)
        {
            uint32_t byte_index = i * bytes_per_line + j / 8;
            uint32_t bit_index = 7 - (j % 8);
            uint8_t pixel = (glyph[byte_index] >> bit_index) & 1;
            if (pixel)
            {
                put_pixel(x, y, color);
            }
            x++;
        }
        x = x_origin;
        y++;
    }
}
