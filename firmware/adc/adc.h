#pragma once
#include <stdint.h>

/* ADC0 channels on J2 (all "side B"). */
#define ADC_STYLUS 21U /* J2.1  P0_29 */

void adc_init(void);

/* One averaged reading: 0 = 0 V ... 4095 = 3.3 V. */
uint16_t adc_read(uint32_t channel);
