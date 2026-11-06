#include "oled.h"
#include "font8x8.h"
#include "i2c/i2c.h"
#include <string.h>

#define ADDR 0x3CU
#define CMD  0x00U /* first byte of a write: "what follows are commands" */
#define DATA 0x40U /* first byte of a write: "what follows are pixels" */

/* The SSD1306's own memory layout: 8 horizontal bands ("pages") of 128 bytes.
 * Each byte is a vertical strip of 8 pixels, bit 0 at the top. 1 bit per pixel = 1 KB. */
static uint8_t frame[OLED_H / 8][OLED_W];

bool oled_init(void)
{
    static const uint8_t init[] = {
        0xAE,       /* display off while configuring */
        0xD5, 0x80, /* internal clock: default */
        0xA8, 0x3F, /* 64 rows */
        0xD3, 0x00, /* no vertical offset */
        0x40,       /* start at memory row 0 */
        0x8D, 0x14, /* charge pump on: makes the ~7 V the OLED needs from 3.3 V */
        0x20, 0x00, /* bytes fill left to right, then wrap to the next page */
        0xA1, 0xC8, /* flip columns and rows so (0,0) is top-left on this module */
        0xDA, 0x12, /* row wiring used by 128x64 panels */
        0x81, 0xCF, /* contrast */
        0xD9, 0xF1, /* pre-charge time */
        0xDB, 0x40, /* "off" pixel voltage level */
        0xA4, 0xA6, /* show memory, not inverted */
        0xAF,       /* display on */
    };
    return i2c_write(ADDR, CMD, init, sizeof init);
}

void oled_clear(void)
{
    memset(frame, 0, sizeof frame);
}

void oled_pixel(int x, int y, bool on)
{
    if (x < 0 || x >= OLED_W || y < 0 || y >= OLED_H)
    {
        return;
    }
    uint8_t bit = 1U << (y % 8);
    if (on)
    {
        frame[y / 8][x] |= bit;
    }
    else
    {
        frame[y / 8][x] &= ~bit;
    }
}

static void block(int x, int y, int size)
{
    for (int dx = 0; dx < size; dx++)
    {
        for (int dy = 0; dy < size; dy++)
        {
            oled_pixel(x + dx, y + dy, true);
        }
    }
}

void oled_text(int x, int y, int scale, const char *s)
{
    for (; *s; s++, x += 8 * scale)
    {
        const uint8_t *glyph = FONT8X8[(*s < 32 || *s > 127) ? 0 : *s - 32];
        for (int col = 0; col < 8; col++)
        {
            for (int row = 0; row < 8; row++)
            {
                if ((glyph[col] >> row) & 1U)
                {
                    block(x + col * scale, y + row * scale, scale);
                }
            }
        }
    }
}

void oled_show(void)
{
    static const uint8_t whole_screen[] = {0x21, 0, OLED_W - 1, 0x22, 0, OLED_H / 8 - 1};
    i2c_write(ADDR, CMD, whole_screen, sizeof whole_screen);
    i2c_write(ADDR, DATA, &frame[0][0], sizeof frame);
}
