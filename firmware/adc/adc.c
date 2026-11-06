#include "adc.h"
#include "fsl_lpadc.h"

void adc_init(void)
{
    lpadc_config_t cfg;

    /* ADC0 boots unclocked: feed it the 48 MHz FRO_HF clock. */
    CLOCK_SetClkDiv(kCLOCK_DivAdc0Clk, 1U);
    CLOCK_AttachClk(kFRO_HF_to_ADC0);
    SPC0->ACTIVE_CFG1 |= 0x1U; /* power the analog reference block, as the SDK examples do */

    LPADC_GetDefaultConfig(&cfg);
    /* Measure against VREFH, which this board ties to the 3.3 V analog supply.
     * The ladder and knobs run from the same 3.3 V, so supply sag cancels out. */
    cfg.referenceVoltageSource  = kLPADC_ReferenceVoltageAlt1;
    cfg.powerLevelMode          = kLPADC_PowerLevelAlt4;
    cfg.enableAnalogPreliminary = true;
    LPADC_Init(ADC0, &cfg);

    LPADC_DoOffsetCalibration(ADC0);
    LPADC_DoAutoCalibration(ADC0);
}

uint16_t adc_read(uint32_t channel)
{
    lpadc_conv_command_config_t cmd;
    lpadc_conv_trigger_config_t trig;
    lpadc_conv_result_t result;

    LPADC_GetDefaultConvCommandConfig(&cmd);
    cmd.channelNumber     = channel;
    cmd.sampleChannelMode = kLPADC_SampleChannelSingleEndSideB;
    /* Connect the ADC's sampling capacitor to the pin for the longest window: 131 clocks
     * (2.7 us). Each sample dumps a little charge onto the pin. A key (<= 3.25 kOhm)
     * swamps it at once, but with no key only the 100 kOhm pull-down drains it, so a
     * short window reads a false "no key" voltage (3 clocks: ~175 counts, 131: ~65).
     * One conversion per read: back-to-back averaging piles the charge up. */
    cmd.sampleTimeMode = kLPADC_SampleTimeADCK131;
    LPADC_SetConvCommandConfig(ADC0, 1U, &cmd);

    LPADC_GetDefaultConvTriggerConfig(&trig);
    trig.targetCommandId = 1U;
    LPADC_SetConvTriggerConfig(ADC0, 0U, &trig);

    LPADC_DoSoftwareTrigger(ADC0, 1U);
    while (!LPADC_GetConvResult(ADC0, &result, 0U))
    {
    }
    return result.convValue >> 3; /* result sits in the top bits of 16; shift to 0..4095 */
}
