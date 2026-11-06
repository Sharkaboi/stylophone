#pragma once
#include <stdbool.h>

/* 0.96" SSD1306 OLED, 128 x 64 pixels, I2C address 0x3C.
 * Draw into memory with the functions below, then oled_show() sends it all. */
#define OLED_W 128
#define OLED_H 64

bool oled_init(void); /* false = nothing answered at 0x3C */
void oled_clear(void);
void oled_pixel(int x, int y, bool on);
void oled_text(int x, int y, int scale, const char *s); /* 8x8 font, each pixel scale x scale */
void oled_show(void);                                     /* ~23 ms at 400 kHz */
