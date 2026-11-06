#include "board.h"
#include "app.h"
#include "fsl_debug_console.h"
#include "adc/adc.h"
#include "keys/keys.h"
#include "i2c/i2c.h"
#include "oled/oled.h"
#include "audio/audio.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static const char *const NOTE[] = {"--", "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4",
                                   "G4", "G#4", "A4", "A#4", "B4", "C5"};

/* Equal temperament: each key is 2^(1/12) higher than the one before; key 1 = C4. */
static float key_hz(int key)
{
    return key ? 261.63f * powf(2.0f, (key - 1) / 12.0f) : 0.0f;
}

static void draw(int key, uint16_t adc)
{
    char line[17];
    snprintf(line, sizeof line, "%dHz adc %u", (int)key_hz(key), adc);

    oled_clear();
    oled_text(0, 0, 1, "Stylophone v1c");
    oled_text((OLED_W - 24 * (int)strlen(NOTE[key])) / 2, 20, 3, NOTE[key]); /* centred, 24 px/char */
    oled_text(0, 56, 1, line);
    oled_show();

    PRINTF("%s\tadc %u\r\n", NOTE[key], adc);
}

int main(void)
{
    BOARD_InitHardware();
    adc_init();
    i2c_init();
    audio_init();
    PRINTF("v1c\r\n");
    i2c_scan();
    if (!oled_init())
    {
        PRINTF("No OLED at 0x3C: check GND, VDD, SCK, SDA\r\n");
    }

    int shown = -1;
    while (1)
    {
        uint16_t adc;
        int key = keys_read(&adc);

        /* Only on a change: every screen update is a 1 KB I2C burst, whose current pulses
         * leak into the audio through the shared ground (heard as crackle). */
        if (key != shown)
        {
            shown = key;
            audio_note(key_hz(key));
            draw(key, adc);
        }
        SDK_DelayAtLeastUs(10000U, SystemCoreClock); /* read 100 times a second */
    }
}
