# Wiring plan

What connects to what, and why. Pins are named by their silkscreen labels (`P0_29`,
`3V3`, `GND`, `5V`). Full table: **Pin map** in `CLAUDE.md`. Parts on hand:
`docs/MCXN236-Stylophone-BOM.csv`. Hands-on technique: `docs/WORKSHOP.md`.

## 1. Power and ground
- Power bank → the **MCU-Link USB-C** port (powers + flashes). Never feed power into
  header pins.
- **`5V`**: DAC VIN, PAM8403.
- **`3V3`**: ladder top, pots, OLED, SD, encoder. This is also the ADC's reference rail,
  which keeps readings ratiometric.
- **`GND`**: any pin labelled GND.
- **Star ground**: digital, DAC analog (AGND) and amp grounds each run their own wire to
  one point. The amp's pulsing current then can't put noise on the DAC's ground.
- **Before every first power-up**: Ω between 5 V and GND. Starting low and climbing =
  capacitors charging, fine. Staying near 0 Ω = short; find it first.

## 2. Keyboard
- **Keys**: 13 pure-nickel strips, 8 × 50 mm, in printed pockets with 2 mm walls
  (128 mm wide). Each key has a wire soldered under one end. Never lacquer the tops:
  a clear coat insulates.
- **Ladder module** (perfboard): `GND–R–K1–R–K2 … R–K13`, with K13 = 3.3 V. That's 13 × 1 kΩ
  joining 14 nodes. A 14-pin header (GND, K1…K12, K13/3V3) carries the key wires and power.
- **Stylus**: brass tip → stranded silicone wire → 3.5 mm plug → PJ311 socket →
  **`P0_29`**, with a **100 kΩ pull-down** and a **100 nF capacitor** to GND at the input.
  The capacitor absorbs the ADC's sampling kickback and filters mains hum (LEARNING #22). No key touched = 0 V.
- Expected: key k = k × 0.254 V (K1 0.25 V … K13 3.30 V), about 315 ADC counts apart.

## 3. Audio
```
SAI1 (I2S) ──► UDA1334A ──► onboard 3.5 mm jack ──► headphones
                   │ Lout
                   └──► 10 kΩ to GND ─► PAM8403 IN ─► L+ ─► toggle ─► speaker ─► L−
```
- **DAC**: VIN ← `5V`, GND, BCLK ← `P3_16`, WSEL ← `P3_17`, DIN ← `P2_8`. Leave the
  SCLK/MUTE/PLL/SF0/SF1/DEEM row open: the defaults are I2S, unmuted, and the PLL locks
  to WSEL, so no MCLK is needed.
- **Headphones**: the DAC's own jack. IEMs are protected by a firmware volume cap; the
  220 Ω resistors can't go into that path. **Never use a PAM8403 for headphones.**
- **Speaker**: PAM8403 on 5 V, one channel. Input from DAC Lout and AGND, with 10 kΩ
  pull-down. The speaker goes **between L+ and L−**: both swing (bridge-tied), so
  neither may touch GND. The toggle sits in series with the speaker; the amp stays
  powered. An unpowered amp with signal on its input can half-power itself through its
  input protection diodes and distort the headphone signal.
- **Aux-in** (optional, v4): PJ311 tip/ring → 10 kΩ each into the amp input node.
- **Volume** is software: volume pot → ADC → sample scaling.

## 4. Controls and display
- **Pots** (4): outer lugs to 3.3 V and GND, wiper to its ADC pin. They are voltage
  dividers, read like the ladder.
- **Encoder** (KY-040, 3.3 V): A, B, push → 3 GPIO.
- **Buttons** (5): each between a GPIO and GND; the chip's internal pull-ups, no
  resistors.
- **OLED** (I2C 0x3C; module pins GND, VDD, SCK, SDA): VDD ← `3V3`, SDA ← `P1_16`,
  SCK ← `P1_17`. The board already has
  4.7 kΩ pull-ups; the module adds its own, which is fine.
- **SD module** (SPI): CS ← `P1_3`, MOSI ← `P1_0`, MISO → `P1_2`, SCK ← `P1_1`,
  `3V3`, GND.
- **RGB LED**: onboard, no wiring.

## 5. Low-noise checklist
1. Short, direct DAC ground to the star point.
2. Keep I2S and analog leads short and away from SPI/SD lines.
3. Amp ground runs separately to the star point.
4. To record on a PC, use **line-in** (or a USB sound card), not mic-in. Hum → a
   ground-loop isolator.
