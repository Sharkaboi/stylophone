#include "keys.h"
#include "adc/adc.h"

/* 13 x 1 kOhm ladder across 3.3 V: key k sits at k/13 of full scale,
 * so the steps are 4095 / 13 = 315 counts apart. The 100 kOhm pull-down makes
 * "no key" read ~0. */
#define STEP         315
#define STABLE_READS 3 /* same answer this many reads in a row = real, not a scrape */

int keys_read(uint16_t *adc)
{
    static int last, same, shown;

    *adc    = adc_read(ADC_STYLUS);
    int now = (*adc + STEP / 2) / STEP; /* nearest step: 0 = no key, 1..13 = key */

    same = (now == last) ? same + 1 : 0;
    last = now;
    if (same >= STABLE_READS)
    {
        shown = now;
    }
    return shown;
}
