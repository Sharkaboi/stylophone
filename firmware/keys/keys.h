#pragma once
#include <stdint.h>

#define KEY_NONE 0 /* keys are 1..13 = C4..C5 */

/* Debounced key under the stylus. Also returns the raw ADC value via *adc. */
int keys_read(uint16_t *adc);
