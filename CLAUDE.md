# CLAUDE.md — FRDM-MCXN236 Stylophone

Full project context. Read it fully before helping. Settled choices: `docs/DECISIONS.md`.
Concepts: `docs/LEARNING.md`. Hands-on technique: `docs/WORKSHOP.md`.

## Who I am and how to work with me
- Developer, strong coding, **near-zero electronics**. Windows + VS Code; dev tools live
  on Z:, C: stays clean. Goal beyond this build: do electronics projects on my own.
- **Teach as you go:** for every step, say what it is, why it works (down to the physics),
  and how to do and verify it. New concepts go into `docs/LEARNING.md`, techniques into
  `docs/WORKSHOP.md`.
- **My learning method:** understand one layer down and treat the rest as a black box;
  run it first, then dissect it; park deep questions; work in ~1 h steps. I'm a
  perfectionist, so keep me moving.
- **Don't guess.** Check the board docs, netlist and parts list before stating anything.
- **Least code:** fewest lines, reading like pseudocode. One folder per feature.
- **Docs:** minimal, bullet points, technical English, no padding.
- **Incremental commits:** one working, tested step per commit. No Claude/AI attribution.
- **Public repo** (github.com/Sharkaboi/stylophone): no WIP, build output, SDK copies,
  machine paths or personal info. Private material goes in `private/` (git-ignored).
- The gadget is standalone. C on the board; no PC-side tools.

## What we're building
A 13-key stylophone on the FRDM-MCXN236. A brass stylus touches nickel keys wired as a
resistor ladder. It has synth tones (square/saw/triangle/sine, sub-oscillator, envelope,
filter, vibrato, echo), optional microSD samples (pitched, or one clip per key), an OLED
with menu and GIF screensaver, 4 knobs, an encoder, 5 buttons, an RGB LED, and
headphone + speaker out. Reference: the original Stylophone (square tone, tuning,
vibrato) plus GEN X-1-style extras. Every module plugs into sockets, so the board can be
reused with zero desoldering. Dropped: soundpad, record/loop, presets.

## Status
- **v0 ✅** Toolchain works; hello_world builds, flashes, prints on **COM3 @ 115200**.
- **v1a code ✅, needs re-test.** `firmware/hello_world.c` reads the ladder on J2.1 and
  prints the note.
  - On the breadboard ladder, the multimeter read ideal taps: 3.30, 3.04 … 0.25 V.
  - The ADC read 4095, 3714, … 315, sagging up to ~11 % in the middle. So the resistors
    are fine, and the sag is on the ADC side.
  - Fixes to try:
    1. Reference `kLPADC_ReferenceVoltageAlt3` (internal VREF) → `Alt1` (VREFH = 3.3 V
       analog supply, so readings are ratiometric).
    2. A longer `sampleTimeMode` (default 3 clocks is short for a source of up to 3.25 kΩ).
    3. Confirm the 100 kΩ pull-down value. Then re-measure.
- **Parts:** all on hand and header pins soldered. Status per part in
  `docs/MCXN236-Stylophone-BOM.csv`. Open checks H1–H8 are in `docs/DECISIONS.md`.

## Proven toolchain (don't re-derive)
- **VS Code + "MCUXpresso for VS Code"**, SDK west flow, board id **`frdmmcxn236`**,
  SDK 26.06 at `z:\frdm_mcxn236\frdm_mcxn236`. Installer components: SDK Developer,
  Arm GNU Toolchain 14.2.1, LinkServer, Config Tools, cmake, ninja, git.
- Python 3.12.1 at `Z:\Python`; extension venv at `~/.mcuxpressotools/.mcux-venv-3.12`.
- Solved gotchas: disable the Windows Store python aliases (exit 9009); Ninja on PATH;
  CMake ≥ 3.30; one complete installer toolchain (don't mix a manual one).
- Machine-local, git-ignored: `firmware/mcux_include.json`,
  `firmware/.vscode/mcuxpresso-tools.json`. README shows how to recreate them.
- Flash + serial via **J10 (MCU-Link USB-C)**. JP6 open = serial on.
- Boot prints: open the serial monitor, then press **SW1 (reset)**.
- Debugging = `PRINTF` to COM3.
- Every MCUXpresso project contains `__repo__` (a junction to the SDK), which is ignored.
  Imported examples (`frdmmcxn236_*/`) are reference only and ignored.
  `frdmmcxn236_sai_interrupt/` is the v1c starting point: its SAI pins and clock match
  our DAC wiring, but it initialises the not-fitted DA7212 codec over I2C and hangs.
  Strip the codec code.

## Board facts (FRDM-MCXN236 rev C, MCXN236VDF, Cortex-M33 150 MHz, FPU + DSP)
- Memory: **1 MB flash, 224 KB RAM + 96 KB SRAMX**, 8 MB QSPI flash.
- Onboard DA7212 codec and J14/J21 audio jacks are **not fitted** → external DAC.
- RGB LED: R=P4_18, G=P4_19, B=P4_17. Buttons: SW1 reset, SW2=P0_20, SW3=P0_6 (ISP).
  SW2 and SW3 work as spare buttons.
- Solder jumpers SJ1–SJ5 ship **1-2 shorted**: J2.6=P1_3, J2.8=P1_0, J1.3=P2_0.
- I2C FC5 has **4.7 kΩ pull-ups on the board** (R6/R7).
- FC3 SPI pins also go to the MCU-Link USB-SPI bridge (R153–R156 fitted).
- Accelerometer links R13/R14/R18 are **not fitted**, so P0_21/P0_23/P0_24 are free.
- **Never use:**
  - P0_25 (light sensor)
  - P4_15/P4_16 (CAN transceiver)
  - P0_16/P0_17 (microphone)
  - P5_7 (USB-C controller)
  - P3_12 (QSPI flash)
- No USB-PD chip: USB-C always delivers 5 V. Power only through USB-C.
- Source of truth: the FRDM-MCXN236 design files (netlist + BOM) and UM12041 from
  nxp.com/design/design-center/development-boards-and-designs/FRDM-MCXN236.
  Per-pin detail: `docs/FRDM-MCXN236-Header-Pinout.md`.

## Pin map (verified against netlist + BOM)
| Function | MCU pin | Header | Notes |
|---|---|---|---|
| Stylus (key ladder) | P0_29 = ADC0_B21 | J2.1 | 100 kΩ pull-down to GND |
| Knob: volume | P0_28 = ADC0_B20 | J2.3 | |
| Knob: tuning | P0_27 = ADC0_B19 | J2.5 | |
| Knob: filter cutoff | P0_26 = ADC0_B18 | J2.7 | |
| Knob: vibrato depth | P0_24 = ADC0_B16 | J2.17 | all 5 analog inputs = ADC0 side B |
| I2S bit clock → DAC BCLK | P3_16 = SAI1_TX_BCLK | J1.1 | |
| I2S word select → DAC WSEL | P3_17 = SAI1_TX_FS | J1.11 | |
| I2S data → DAC DIN | P2_8 = SAI1_TXD0 | J1.5 | MCLK unused |
| I2C SDA (OLED 0x3C) | P1_16 = FC5 | J2.18 | |
| I2C SCL | P1_17 = FC5 | J2.20 | |
| SPI MOSI (SD) | P1_0 = FC3_P0 | J2.8 | |
| SPI SCK | P1_1 = FC3_P1 | J2.12 | |
| SPI MISO | P1_2 = FC3_P2 | J2.10 | |
| SPI CS | P1_3 = FC3_P3 | J2.6 | |
| Encoder A | P0_23 | J2.2 | |
| Encoder B | P3_14 | J2.4 | |
| Encoder push | P0_22 | J2.9 | |
| Button octave − | P2_0 | J1.6 | |
| Button octave + | P0_21 | J1.10 | |
| Button mode | P2_7 | J1.12 | |
| Button sound | P3_18 | J1.9 | |
| Button screen | P1_7 | J1.13 | |
| Spare GPIO | P2_9 J1.15 · P4_6 J4.2 · P4_12 J4.10 · P4_13 J4.12 · P0_18 J4.5 · P4_21 J3.3 | | |
| 5 V | J3.10 | | DAC VIN, PAM8403 |
| 3.3 V | J3.4, J3.8, J4.1 | | ladder, knobs, OLED, SD, encoder |
| GND | J2.14, J3.12, J3.14, J4.3 | | |

Buttons and encoder: switch to GND, internal pull-up, no resistors.

## Keyboard
- 13 pure-nickel keys, 8 × 50 mm, in printed pockets with 2 mm walls (128 mm wide).
  **Never lacquer the key tops:** a clear coat insulates.
- Ladder on a perfboard module: `GND–R–K1 … R–K13`, with K13 = 3.3 V; 13 × 1 kΩ 1 %.
  Key k = k × 0.254 V. 100 kΩ pull-down at the stylus input.
- One stylus + one ADC = one key at a time. The firmware uses 2 voices so tails overlap.

## Audio
- SAI1 as I2S master, **32 kHz 16-bit stereo**, no MCLK, feeding the UDA1334A
  (VIN 5 V; config pins open).
- Samples are rendered in the SAI interrupt (budget in DECISIONS D4).
- Headphones: the DAC's own jack, with a firmware volume cap protecting IEMs.
  **Never PAM8403 → headphones.**
- Speaker: DAC Lout → 10 kΩ pull-down → PAM8403 (5 V) → L+ / toggle / speaker / L−.
  **Bridge-tied: speaker never to GND.**
- Volume is software (knob → ADC → scale).

## Build order (each stage tested before the next)
- v0 ✅
- **v1a** ladder → ADC → note (re-test with fixes)
- v1b OLED: I2C scan, text, note name
- v1c sound: strip the codec from `sai_interrupt`, sine to headphones
- v1d instrument: keys play notes; octave buttons, RGB LED, volume and tuning knobs.
  Milestone: playable.
- v2 synth: waveforms, sub-oscillator, envelope, filter, vibrato, echo, encoder menu
- v3 microSD samples (FatFs + SPI driver) and GIF screensaver
- v4 build: nickel key plate, ladder module, carrier perfboard, speaker stage,
  OpenSCAD enclosure

## Firmware shape
- Setup once, then `while (1) { read inputs → decide → update outputs }`.
- Audio is the exception: samples are produced in the SAI interrupt, never in the loop.
  The loop never blocks longer than the audio buffer (OLED and SD in chunks).
- I2S format mismatch is the classic "no sound" trap (I2S, 16-bit, stereo).
- Debounce buttons and keys.

## Design for reuse and wear
- Solder the cheap side, plug the expensive side. Panel parts use half-jumper leads.
- Wear order: stylus cable (strain relief, removable plug) → stressed joints (hot glue)
  → key tarnish (polish; never lacquer contact faces) → modules last.

## Cautions
1. Never use the pins listed under "Never use".
2. FC3 SPI is shared with the MCU-Link bridge, the first suspect if SD is flaky.
3. Star ground. 4. Never ground a PAM8403 output; never drive headphones from one.
5. Feed a PC line-in, not mic-in. 6. The breadboard hisses; perfboard is quieter.

## Files
- `README.md`: setup.
- `docs/DECISIONS.md`: settled choices + open checks.
- `docs/LEARNING.md`, `docs/WORKSHOP.md`.
- `docs/MCXN236-Stylophone-Wiring-Plan.md`, `docs/FRDM-MCXN236-Header-Pinout.md`.
- `docs/MCXN236-Stylophone-BOM.csv`: parts on hand and their status.
