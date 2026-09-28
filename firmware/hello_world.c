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
/* Pot wiper -> J2.1 = P0_29 = ADC0_B21, i.e. channel 21 on the B side.
 * The driver default is side A, so sampleChannelMode must be set. */
#define APP_ADC_CHANNEL 21U
#define APP_ADC_CMDID   1U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    lpadc_config_t adcConfig;
    lpadc_conv_command_config_t cmd;
    lpadc_conv_trigger_config_t trig;
    lpadc_conv_result_t result;

    /* Init board hardware. */
    BOARD_InitHardware();

    /* ADC0 and its voltage reference both boot powered down. */
    CLOCK_SetClkDiv(kCLOCK_DivAdc0Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_ADC0);
    SPC0->ACTIVE_CFG1 |= 0x1U;

    PRINTF("MCUX SDK version: %s\r\n", MCUXSDK_VERSION_FULL_STR);

    PRINTF("hello world.\r\n");

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
        PRINTF("ADC = %u\r\n", (unsigned)(result.convValue >> 3));
        SDK_DelayAtLeastUs(200000, SystemCoreClock);
    }
}
