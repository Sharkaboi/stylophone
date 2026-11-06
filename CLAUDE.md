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
- **Name board pins by their silkscreen label** (`P0_29`, `3V3`, `GND`, `5V`), never by
  header number (`J2.1`); the header numbers aren't printed on the board.
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
- **v1a ✅** `firmware/main.c` + `adc/` + `keys/`: the ladder on `P0_29` → key → serial,
  printed 4×/s (`note  adc N  expect M`).
  - ADC settings: reference VREFH (`Alt1`), 131-clock sample window, one conversion per
    read, no averaging.
  - Measured: keys within −4 … −76 counts of k × 315 (the 100 kΩ load, worst mid-ladder);
    no key ≈ 60. Decode = nearest step, `(adc + 157) / 315`.
  - Holding the stylus's bare metal injects mains hum (0–700): fingers off the tip. A
    100 nF capacitor at the input is planned.
  - Command-line flashing fails while a VS Code debug session is attached.
- **v1b ✅** `i2c/` (FC5 on `P1_16`/`P1_17`, 400 kHz, bus scan at boot) + `oled/` (SSD1306
  driver, 1 KB frame buffer, unscii 8×8 font, text at any scale). The screen shows the
  title, the note at 3× size, and the ADC value. The scan finds 0x3c; all 13 notes show in order.
  - With the OLED on the same breadboard + rail as the ladder, top keys read ~2 % low (the
    jumper's voltage drop). Fix: separate `3V3`/`GND` jumpers for the ladder.
    Idle rose to ~95 (I2C bursts disturbing ground), still under 157.
- **v1c ✅** `audio/`: SAI1 as I2S master at 32 kHz, 16-bit stereo.
  - Clock: PLL1 = 24.576 MHz (FRO 48 MHz / 25 × 256 / 20), BCLK 1.024 MHz.
  - Pins `P3_16`/`P3_17`/`P2_8`, pin function 10.
  - The FIFO-request interrupt refills 2 frames, 16k times a second.
  - Sine table with a phase-accumulator oscillator and a 5 ms fade in/out.
  - Each key plays its equal-temperament pitch.
  - Heard on the speaker (DAC Lout → PAM8403 → 8 Ω) at `LEVEL_MAX` 512 (1/64, ~6 mW).
  - Breadboard power: top rails `3V3` (OLED); bottom rails `P5V0` (DAC VIN, amp), each
    with its own `GND` jumper. DAC VIN reads 4.6 V.
  - Debug lessons: a DAC fed from 3V3 was back-powered through its I2S pins (LEARNING #25).
    A bare small speaker hardly radiates low notes (#26).
  - Headphones/IEMs not tried yet: measure the level before they go near ears.
- **Next: v1d**: octave buttons, RGB LED per note, volume + tuning knobs.
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
- Command-line build (Git Bash). Put the installer's CMake 3.30 first: `Z:\CMake` is an
  old 3.28 that breaks reconfigures.
  ```
  cd firmware
  export PATH="$HOME/.mcuxpressotools/ninja-1.13.2:$HOME/.mcuxpressotools/cmake-3.30.0-windows-x86_64/bin:$PATH"
  cmake --preset debug && cmake --build debug        # → debug/firmware.elf
  /c/NXP/LinkServer_26.6.137/LinkServer.exe flash MCXN236:FRDM-MCXN236 load debug/firmware.elf
  ```
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
- No USB-PD chip: USB-C always delivers 5 V. Power only through USB-C, through an inline
  on/off switch on the 5 V wire.
- Mounting: 4 unplated 3.2 mm holes (BH1–BH4, M3), probably under the rubber feet.
- Source of truth: the FRDM-MCXN236 design files (netlist + BOM) and UM12041 from
  nxp.com/design/design-center/development-boards-and-designs/FRDM-MCXN236.
  Per-pin detail: `docs/FRDM-MCXN236-Header-Pinout.md`.

## Pin map (verified against netlist + BOM)
Use the "MCU pin" name: it's what the underside silkscreen shows. A label printed on two
pins is the same signal, so either works. The Header column is for cross-checking docs.
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
| I2C SDA (OLED 0x3C) | P1_16 = FC5 | J2.18 | OLED pins: GND, VDD, SCK, SDA |
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
| 5 V (label `P5V0`) | J3.10 | | DAC VIN, PAM8403. Power row: –, 3V3, RESET, 3V3, **P5V0**, GND, GND, VIN |
| 3.3 V (label `3V3`) | J3.4, J3.8, J4.1 | | ladder, knobs, OLED, SD, encoder |
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
- Screen updates are an audio noise source (I2C current bursts through shared ground,
  LEARNING #27). Redraw only on change until the perfboard has star ground + decoupling.
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
