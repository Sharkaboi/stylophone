#include "i2c.h"
#include "fsl_lpi2c.h"
#include "fsl_port.h"
#include "fsl_debug_console.h"

void i2c_init(void)
{
    lpi2c_master_config_t cfg;

    /* Hand both pins to FlexComm 5 (pin function 2). Keep their input buffers on: I2C
     * reads its own lines back to see acknowledgements. Pull-ups: 4.7 kOhm on the board. */
    PORT_SetPinMux(PORT1, 16U, kPORT_MuxAlt2); /* SDA */
    PORT_SetPinMux(PORT1, 17U, kPORT_MuxAlt2); /* SCL */
    PORT1->PCR[16] |= PORT_PCR_IBE_MASK;
    PORT1->PCR[17] |= PORT_PCR_IBE_MASK;

    CLOCK_SetClkDiv(kCLOCK_DivFlexcom5Clk, 1U);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM5);

    LPI2C_MasterGetDefaultConfig(&cfg);
    cfg.baudRate_Hz = 400000U;
    LPI2C_MasterInit(LPI2C5, &cfg, CLOCK_GetLPFlexCommClkFreq(5U));
}

bool i2c_write(uint8_t addr, uint8_t prefix, const uint8_t *data, size_t len)
{
    lpi2c_master_transfer_t xfer = {
        .slaveAddress   = addr,
        .direction      = kLPI2C_Write,
        .subaddress     = prefix,
        .subaddressSize = 1U,
        .data           = (void *)data,
        .dataSize       = len,
    };
    return LPI2C_MasterTransferBlocking(LPI2C5, &xfer) == kStatus_Success;
}

void i2c_scan(void)
{
    PRINTF("I2C devices:");
    for (uint8_t addr = 0x08U; addr < 0x78U; addr++) /* the valid 7-bit address range */
    {
        lpi2c_master_transfer_t xfer = {.slaveAddress = addr, .direction = kLPI2C_Write};
        if (LPI2C_MasterTransferBlocking(LPI2C5, &xfer) == kStatus_Success)
        {
            PRINTF(" 0x%x", addr);
        }
    }
    PRINTF("\r\n");
}
