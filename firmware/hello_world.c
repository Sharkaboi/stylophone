/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_lpadc.h"
#include "board.h"
#include "app.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_ADC_CHANNEL 21U
#define APP_ADC_CMDID   1U
#define APP_NO_KEY_MAX  150U
#define APP_NO_KEY      (-1)
#define APP_DEBOUNCE    3U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const uint16_t kKeyAdc[] = {315U,  604U,  877U,  1138U, 1401U, 1675U, 1955U,
                                   2250U, 2553U, 2892U, 3277U, 3714U, 4095U};
static const char *const kKeyNote[] = {"C4", "C#4", "D4", "D#4", "E4",  "F4", "F#4",
                                       "G4", "G#4", "A4", "A#4", "B4",  "C5"};
#define APP_KEY_COUNT (sizeof(kKeyAdc) / sizeof(kKeyAdc[0]))

/*******************************************************************************
 * Code
 ******************************************************************************/
static int app_classify(uint32_t adc)
{
    uint32_t bestDist = 0xFFFFFFFFU;
    uint32_t d, i;
    int best = APP_NO_KEY;

    if (adc <= APP_NO_KEY_MAX)
    {
        return APP_NO_KEY;
    }
    for (i = 0U; i < APP_KEY_COUNT; i++)
    {
        d = (adc > kKeyAdc[i]) ? (adc - kKeyAdc[i]) : (kKeyAdc[i] - adc);
        if (d < bestDist)
        {
            bestDist = d;
            best     = (int)i;
        }
    }
    return best;
}

/*!
 * @brief Main function
 */
int main(void)
{
    lpadc_config_t adcConfig;
    lpadc_conv_command_config_t cmd;
    lpadc_conv_trigger_config_t trig;
    lpadc_conv_result_t result;
    int reported   = APP_NO_KEY;
    int candidate  = APP_NO_KEY;
    uint32_t agree = 0U;
    int key;

    /* Init board hardware. */
    BOARD_InitHardware();

    CLOCK_SetClkDiv(kCLOCK_DivAdc0Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_ADC0);
    SPC0->ACTIVE_CFG1 |= 0x1U;

    PRINTF("MCUX SDK version: %s\r\n", MCUXSDK_VERSION_FULL_STR);

    PRINTF("13-key ladder on J2.1 - C4 to C5\r\n");

    LPADC_GetDefaultConfig(&adcConfig);
    adcConfig.enableAnalogPreliminary = true;
    adcConfig.powerLevelMode          = kLPADC_PowerLevelAlt4;
    adcConfig.referenceVoltageSource  = kLPADC_ReferenceVoltageAlt3;
    LPADC_Init(ADC0, &adcConfig);

    LPADC_DoOffsetCalibration(ADC0);
    SDK_DelayAtLeastUs(1U, SystemCoreClock);
    LPADC_DoAutoCalibration(ADC0);

    LPADC_GetDefaultConvCommandConfig(&cmd);
    cmd.channelNumber     = APP_ADC_CHANNEL;
    cmd.sampleChannelMode = kLPADC_SampleChannelSingleEndSideB;
    LPADC_SetConvCommandConfig(ADC0, APP_ADC_CMDID, &cmd);

    LPADC_GetDefaultConvTriggerConfig(&trig);
    trig.targetCommandId = APP_ADC_CMDID;
    LPADC_SetConvTriggerConfig(ADC0, 0U, &trig);

    while (1)
    {
        LPADC_DoSoftwareTrigger(ADC0, 1U);
        while (!LPADC_GetConvResult(ADC0, &result, 0U))
        {
        }
        key = app_classify(result.convValue >> 3);

        if (key != candidate)
        {
            candidate = key;
            agree     = 0U;
        }
        else if (agree < APP_DEBOUNCE)
        {
            agree++;
        }

        if ((agree >= APP_DEBOUNCE) && (candidate != reported))
        {
            reported = candidate;
            PRINTF("%s\r\n", (reported == APP_NO_KEY) ? "--" : kKeyNote[reported]);
        }

        SDK_DelayAtLeastUs(10000, SystemCoreClock);
    }
}
