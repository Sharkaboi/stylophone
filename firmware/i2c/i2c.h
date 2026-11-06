#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* I2C bus on FlexComm 5: SDA = P1_16, SCL = P1_17, 400 kHz. */
void i2c_init(void);

/* Send `prefix` then `len` bytes to the device at `addr`. True = the device answered. */
bool i2c_write(uint8_t addr, uint8_t prefix, const uint8_t *data, size_t len);

/* Print every address that answers (a wiring check). */
void i2c_scan(void);
