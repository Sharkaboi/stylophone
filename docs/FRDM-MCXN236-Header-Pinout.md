# FRDM-MCXN236 header pinout

Generated from the board's own netlist (`pstxnet.dat`, SCH-90828 rev C) inside
`FRDM-MCXN236-DESIGN-FILES.zip`. This is the design data itself, not a transcription,
so it cannot disagree with the board.

## Finding pin 1 physically

- **Pin 1 has a square pad**; every other pad is round. Look at the solder side.
- Odd pins (1,3,5...) are one row, even pins (2,4,6...) the other.
- Headers are identified by pin count: **J2=20, J1=16, J3=16, J4=12**.

## Quick reference

| Need | Where |
|---|---|
| **3.3V** (`VDD_BOARD`) | J3.4, J3.8, **J4.1** |
| **5V** (`P5V0`) | J3.10 |
| **GND** | **J2.14**, J3.12, J3.14, J4.3 |
| **ADC, free** | **J2.1, J2.3, J2.5, J2.7** |
| ADC, NOT free | J2.11 - light sensor Q2 via R38 |
| I2C (FC5) | J2.18 SDA, J2.20 SCL |
| Do not use | J2.16 `VDDA_MCU`, J2.19 `VREFO` |

`Also on` = other parts sharing the net. **Non-empty means the pin is not exclusively yours.**

## J1 - 16 pins (8 x 2)

| Pin | Signal | MCU ball | Also on |
|----:|--------|:--------:|---------|
| 1 | `P3_16` | J15 | SJ2.3, J3.7, R106.1 |
| 2 | `P4_3` | U1 | J5.3, R56.2 |
| 3 | `N1499583` | - | SJ3.2 |
| 4 | `P4_2` | T1 | J5.4, R55.2 |
| 5 | `P2_8/SAI1_TXD0` | M2 | R109.1 |
| 6 | `P2_0` | H2 | SJ3.1 |
| 7 | `P3_6/SAI1_MCLK` | D17 | R103.2 |
| 8 | `P3_12/PWM1_A0` | - | J3.15, R37.2 |
| 9 | `P3_18/SAI1_RX_BCLK` | K16 |  |
| 10 | `P0_21` | A8 | J8.14, R13.1 |
| 11 | `P3_17` | K15 | J3.5, SJ3.3, R108.1 |
| 12 | `P2_7` | L2 | J3.13 |
| 13 | `P1_7/SAI1_RX_FS` | - | R22.2 |
| 14 | `P3_17` | K15 | J3.5, SJ3.3, R108.1 |
| 15 | `P2_9/SAI1_RXD0` | M1 | R107.1 |
| 16 | `P0_22` | B8 | J2.9 |

## J2 - 20 pins (10 x 2)

| Pin | Signal | MCU ball | Also on |
|----:|--------|:--------:|---------|
| 1 | `P0_29/ADC0_B21-MC_BEMF_A` | F8 |  |
| 2 | `P0_23` | B7 | J8.5, R18.1 |
| 3 | `P0_28/ADC0_B20-MC_BEMF_B` | E8 |  |
| 4 | `P3_14` | H17 | J3.11 |
| 5 | `P0_27/ADC0_B19-MC_BEMF_C` | E10 |  |
| 6 | `N1499275` | - | SJ1.2 |
| 7 | `P0_26/ADC0_B18-MC_VOLT_DCB` | F10 |  |
| 8 | `N1499293` | - | SJ2.2 |
| 9 | `P0_22` | B8 | J1.16 |
| 10 | `P1_2` | C4 | J7.5, R154.1, J6.5 |
| 11 | `P0_25/ADC0_B17-LIGHT_SENSOR` | A6 | R38.2 |
| 12 | `P1_1` | C5 | J7.7, R153.1, J6.4 |
| 13 | `NC` | U10 | _not connected_ |
| 14 | `GND` | P16 | _ground rail_ |
| 15 | `NC` | U10 | _not connected_ |
| 16 | `VDDA_MCU` | R5 | _MCU analog supply - do not load_ |
| 17 | `P0_24` | B6 | J8.8, R14.1 |
| 18 | `P1_16/FC5_I2C_SDA-ARD_D18` | - | R39.2, R6.2 |
| 19 | `ANA_7/VREFO` | U4 | C29.1 |
| 20 | `P1_17/FC5_I2C_SCL-ARD_D19` | - | R40.2, R7.2 |

## J3 - 16 pins (8 x 2)

| Pin | Signal | MCU ball | Also on |
|----:|--------|:--------:|---------|
| 1 | `P4_13/TRIG_IN8-MC_ENC_B` | - | R57.2 |
| 2 | `NC` | U10 | _not connected_ |
| 3 | `P4_21/TRIG_IN9-MC_ENC_A` | T11 |  |
| 4 | `VDD_BOARD` | - | _3.3V rail_ |
| 5 | `P3_17` | K15 | J1.14, J1.11, SJ3.3, R108.1 |
| 6 | `MCU_RESET_B` | F3 | C86.1, R161.2, C87.1, SW1.4, D12.C _(+5 more)_ |
| 7 | `P3_16` | J15 | SJ2.3, J1.1, R106.1 |
| 8 | `VDD_BOARD` | - | _3.3V rail_ |
| 9 | `P3_15` | H15 | SJ1.3 |
| 10 | `P5V0` | - | _5V rail_ |
| 11 | `P3_14` | H17 | J2.4 |
| 12 | `GND` | P16 | _ground rail_ |
| 13 | `P2_7` | L2 | J1.12 |
| 14 | `GND` | P16 | _ground rail_ |
| 15 | `P3_12/PWM1_A0` | - | J1.8, R37.2 |
| 16 | `P5-9V_VIN` | - | _VIN (5-9V)_ |

## J4 - 12 pins (6 x 2)

| Pin | Signal | MCU ball | Also on |
|----:|--------|:--------:|---------|
| 1 | `VDD_BOARD` | - | _3.3V rail_ |
| 2 | `P4_6` | N7 | J8.6 |
| 3 | `GND` | P16 | _ground rail_ |
| 4 | `P4_15` | T8 | R25.1 |
| 5 | `P0_18` | C10 | SJ5.3 |
| 6 | `P4_16` | R8 | R92.1, R67.1 |
| 7 | `P0_16/PDM0_CLK-DMIC` | - | U7.2, R96.1, R5.2 |
| 8 | `P4_17` | R9 | R169.2, R88.1 |
| 9 | `P0_17/PDM0_DATA0-DMIC` | - | R99.1, U7.1, R4.2 |
| 10 | `P4_12/ARD_A4` | T6 | R299.2 |
| 11 | `P5_7` | L13 | R257.2, U41.6 |
| 12 | `P4_13` | T7 | R298.2, R57.1 |
