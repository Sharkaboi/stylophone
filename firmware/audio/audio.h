#pragma once

/* Sound out to the UDA1334A DAC over I2S: 32 kHz, 16-bit, stereo.
 * BCLK = P3_16, WSEL = P3_17, DIN = P2_8. */
#define AUDIO_RATE 32000

void audio_init(void);

/* Play a tone at hz (fades in over 5 ms). 0 = fade out. */
void audio_note(float hz);
