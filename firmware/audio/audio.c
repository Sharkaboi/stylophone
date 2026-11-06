#include "audio.h"
#include "fsl_sai.h"
#include "fsl_port.h"
#include <math.h>

/* Loudness, out of 32768 = full scale. The DAC's full scale is ~0.9 V rms and the PAM8403
 * amplifies ~16x, so the speaker gets P = V^2 / R:
 *   2048 (1/16)  → 0.9 V rms  → ~0.1 W   (clearly loud)
 *   512  (1/64)  → 0.22 V rms → ~6 mW    (soft)
 *   128  (1/256) → 0.06 V rms → ~0.4 mW  (cone moves, barely audible: a bare small
 *                                         speaker hardly radiates low notes)
 * The volume knob (v1d) will scale this. 2048 is too loud for IEMs on the DAC jack. */
#define LEVEL_MAX 2048
#define RAMP      MAX(LEVEL_MAX / (AUDIO_RATE / 200), 1) /* full level in ~5 ms: no click */

static int16_t sine[256];        /* one cycle of a sine wave */
static volatile uint32_t step;   /* how far through the cycle each sample moves */
static volatile int gate;        /* 1 = note on, 0 = fading out */

void audio_init(void)
{
    sai_transceiver_t cfg;

    for (int i = 0; i < 256; i++)
    {
        sine[i] = (int16_t)(32767.0f * sinf(2.0f * 3.14159265f * i / 256.0f));
    }

    /* Pins: route P3_16, P3_17, P2_8 to SAI1 (pin function 10). */
    CLOCK_EnableClock(kCLOCK_Port2);
    CLOCK_EnableClock(kCLOCK_Port3);
    PORT_SetPinMux(PORT3, 16U, kPORT_MuxAlt10); /* bit clock */
    PORT_SetPinMux(PORT3, 17U, kPORT_MuxAlt10); /* word select: left/right */
    PORT_SetPinMux(PORT2, 8U, kPORT_MuxAlt10);  /* data */

    /* Audio clock: PLL1 makes 24.576 MHz from the 48 MHz FRO
     * (48 / 25 x 256 / 20). 24.576 MHz / 32 kHz / 32 bits per frame = 24, a whole number,
     * so the sample rate is exact. */
    const pll_setup_t pll1 = {
        .pllctrl = SCG_SPLLCTRL_SOURCE(1U) | SCG_SPLLCTRL_SELI(3U) | SCG_SPLLCTRL_SELP(1U),
        .pllndiv = SCG_SPLLNDIV_NDIV(25U),
        .pllpdiv = SCG_SPLLPDIV_PDIV(10U),
        .pllmdiv = SCG_SPLLMDIV_MDIV(256U),
        .pllRate = 24576000U,
    };
    CLOCK_EnableClock(kCLOCK_Scg);
    CLOCK_SetPLL1Freq(&pll1);
    CLOCK_SetClkDiv(kCLOCK_DivPLL1Clk0, 1U);
    CLOCK_SetClkDiv(kCLOCK_DivSai1Clk, 1U);
    CLOCK_AttachClk(kPLL1_CLK0_to_SAI1);

    /* SAI1 as I2S master: standard I2S framing, 16-bit words, left + right. */
    SAI_Init(SAI1);
    SAI_GetClassicI2SConfig(&cfg, kSAI_WordWidth16bits, kSAI_Stereo, 1U << 0);
    cfg.masterSlave = kSAI_Master;
    SAI_TxSetConfig(SAI1, &cfg);
    SAI_TxSetBitClockRate(SAI1, CLOCK_GetSaiClkFreq(1U), AUDIO_RATE, kSAI_WordWidth16bits, 2U);

    sai_master_clock_t mclk = {.mclkOutputEnable = true, .mclkHz = 24576000U, .mclkSourceClkHz = 24576000U};
    SAI_SetMasterClockConfig(SAI1, &mclk); /* internal master clock; the DAC doesn't need the pin */

    /* Interrupt whenever the 8-word output FIFO drops to half full. */
    EnableIRQ(SAI1_IRQn);
    SAI_TxEnableInterrupts(SAI1, kSAI_FIFORequestInterruptEnable | kSAI_FIFOErrorInterruptEnable);
    SAI_TxEnable(SAI1, true);
}

void audio_note(float hz)
{
    if (hz > 0.0f)
    {
        step = (uint32_t)(hz / AUDIO_RATE * 4294967296.0f); /* fraction of a cycle, in 2^32ths */
    }
    gate = hz > 0.0f;
}

/* One sample: walk through the sine table, scaled by a level that ramps up or down. */
static int16_t next_sample(void)
{
    static uint32_t phase;
    static int32_t level;

    int32_t target = gate ? LEVEL_MAX : 0;
    level = (level < target) ? MIN(level + RAMP, target) : MAX(level - RAMP, target);
    phase += step;
    return (int16_t)(sine[phase >> 24] * level / 32768);
}

/* Runs 16,000 times a second: refill 2 frames (left + right words) into the FIFO. */
void SAI1_IRQHandler(void)
{
    if (SAI_TxGetStatusFlag(SAI1) & kSAI_FIFOErrorFlag) /* we were too late once */
    {
        SAI_TxClearStatusFlags(SAI1, kSAI_FIFOErrorFlag);
    }
    for (int frame = 0; frame < 2; frame++)
    {
        uint16_t s = (uint16_t)next_sample();
        SAI_WriteData(SAI1, 0U, s); /* left */
        SAI_WriteData(SAI1, 0U, s); /* right */
    }
    SDK_ISR_EXIT_BARRIER;
}
