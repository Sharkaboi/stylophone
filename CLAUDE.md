# CLAUDE.md — FRDM-MCXN236 Stylophone + Soundpad

Full project context. Read it fully before helping. Open choices live in
`docs/DECISIONS.md`; physics/how-it-works notes in `docs/LEARNING.md`.

## Who I am and how to work with me
- Developer, strong coding, **near-zero electronics**. Windows + VS Code.
  Goal beyond this build: design electronics projects (any kind) on my own.
- **Teach as you go.** For every hardware or firmware step explain *what* it is,
  *why* it works (down to the physics), and *how* to do and verify it. Every new
  concept gets a short entry in `docs/LEARNING.md`. Hand-hold on hardware.
- **Least code.** Fewest lines, reading like pseudocode. No abstraction layers,
  no cleverness, one small file per job.
- **Incremental commits.** One working, tested step per commit, clear message.
  No Claude/AI co-author or attribution lines in commits or PRs.
- **Public repo** (github.com/Sharkaboi/stylophone). Never commit WIP/temp, build
  output, SDK copies, secrets or personal info. Private material → `private/` (ignored).
- C on the board. Python for PC-side tools in `tools/`.
- Keep responses tight, no fluff.

## What we're building
A 13-key **stylophone + soundpad** on the FRDM-MCXN236: stylus on a copper key
strip, capacitive drum pads, OLED, 4 knobs (volume / tuning / filter cutoff / LFO
depth), menu encoder, 5 buttons, headphone + speaker out, sample playback,
record/loop, presets. Boutique-DIY finish. Every module plugs into sockets so the
board can be reused later with zero desoldering.

## Status
- **v0 ✅** Toolchain works; hello_world builds, flashes, prints on **COM3 @ 115200**.
- **v1a code ✅, needs re-test.** `firmware/hello_world.c` reads the ladder on J2.1
  and prints the note. Two open issues:
  1. Reference is `kLPADC_ReferenceVoltageAlt3` = the internal VREF block (per the
     SDK example comment), not VREFH, which this board ties to the 3.3 V analog
     supply. Readings only stay ratiometric with the ladder if the reference is that
     same rail → switch to `Alt1`.
  2. The measured key table is right at both ends (K1 315, K13 4095) but sags up to
     ~11 % mid-ladder. That shape follows the tap's source resistance (largest in
     the middle), which points at loading, not the reference. Suspects: ADC sample
     time too short (default 3 clocks), or the pull-down isn't really 100 kΩ.
     Try a longer `sampleTimeMode`, measure the pull-down, then re-measure the keys.
- **Now:** settling every item in `docs/DECISIONS.md`, then implementing.

## Proven toolchain (don't re-derive)
- **VS Code + "MCUXpresso for VS Code"**, SDK west flow, board id **`frdmmcxn236`**,
  SDK 26.06 at `z:\frdm_mcxn236\frdm_mcxn236`. Installer components: SDK Developer,
  Arm GNU Toolchain 14.2.1, LinkServer, Config Tools, cmake, ninja, git.
- Python 3.12.1; extension venv at `~/.mcuxpressotools/.mcux-venv-3.12`.
- Solved gotchas: disable Windows Store python aliases (exit 9009); Ninja on PATH;
  CMake ≥ 3.30; keep one complete toolchain on Z: (don't mix manual + installer).
- Flash + serial via **J10 (MCU-Link USB-C)**. JP6 open = serial on. J11 = MCU USB.
- Boot prints: open serial monitor, then press **SW1 (reset)**.
- Debugging = `PRINTF` to COM3.
- Each MCUXpresso project has `__repo__` (junction to the SDK) — ignored by git.
- SDK examples imported at repo root (`frdmmcxn236_*/`) are reference only, ignored.
  `frdmmcxn236_sai_interrupt/` is there now: its SAI pins/clock match our DAC
  wiring, but it initialises the not-fitted DA7212 codec and will hang — strip that.

## Board facts (FRDM-MCXN236 rev C, MCXN236VDF, Cortex-M33 150 MHz)
- Memory: **1 MB flash, 224 KB RAM + 96 KB SRAMX**, 8 MB QSPI flash (W25Q64).
- Onboard DA7212 codec + J14/J21 audio jacks are **not fitted** → external DAC.
- RGB LED R=P4_18 G=P4_19 B=P4_17. SW1 reset, SW2=P0_20, SW3=P0_6 (ISP) —
  SW2/SW3 are usable as spare buttons.
- Solder jumpers SJ1–SJ5 ship **1-2 shorted**: J2.6=P1_3, J2.8=P1_0, J1.3=P2_0.
- I2C FC5 has **4.7 kΩ pull-ups on the board** (R6/R7).
- FC3 SPI pins also go to the MCU-Link USB-SPI bridge (R153–R156, 0 Ω, fitted).
- Accelerometer interrupt links R13/R14/R18 are **not fitted** → P0_21/P0_23/P0_24 free.
- P0_25 (J2.11) is wired to the light sensor (R38 fitted) → not usable as ADC.
- P4_15/P4_16 go to the CAN transceiver, P0_16/P0_17 to the digital mic,
  P5_7 to the USB-C controller, P3_12 to QSPI → never use.
- Source of truth: netlist + BOM inside `nxp-docs/FRDM-MCXN236-DESIGN-FILES.zip`;
  per-pin detail in `docs/FRDM-MCXN236-Header-Pinout.md`.

## Pin map (verified against netlist + BOM)
| Function | MCU pin | Header | Notes |
|---|---|---|---|
| Stylus (key ladder) | P0_29 = ADC0_B21 | J2.1 | 100 kΩ pull-down to GND |
| Pot: volume | P0_28 = ADC0_B20 | J2.3 | |
| Pot: tuning | P0_27 = ADC0_B19 | J2.5 | |
| Pot: filter cutoff | P0_26 = ADC0_B18 | J2.7 | |
| Pot: LFO depth | P0_24 = ADC0_B16 | J2.17 | all 5 analog inputs = ADC0 side B |
| I2S bit clock → DAC BCLK | P3_16 = SAI1_TX_BCLK | J1.1 | |
| I2S word select → DAC WSEL | P3_17 = SAI1_TX_FS | J1.11 | |
| I2S data → DAC DIN | P2_8 = SAI1_TXD0 | J1.5 | MCLK (P3_6, J1.7) unused |
| I2C SDA (OLED 0x3C, MPR121 0x5A) | P1_16 = FC5 | J2.18 | |
| I2C SCL | P1_17 = FC5 | J2.20 | |
| SPI MOSI (SD, if used) | P1_0 = FC3_P0 | J2.8 | |
| SPI SCK | P1_1 = FC3_P1 | J2.12 | |
| SPI MISO | P1_2 = FC3_P2 | J2.10 | |
| SPI CS | P1_3 = FC3_P3 | J2.6 | |
| Encoder A | P0_23 | J2.2 | |
| Encoder B | P3_14 | J2.4 | |
| Encoder push | P0_22 | J2.9 | |
| Button octave − | P2_0 | J1.6 | |
| Button octave + | P0_21 | J1.10 | |
| Button mode | P2_7 | J1.12 | |
| Button preset | P3_18 | J1.9 | |
| Button record | P1_7 | J1.13 | |
| Spare GPIO | P2_9 J1.15 · P4_6 J4.2 · P4_12 J4.10 · P4_13 J4.12 · P0_18 J4.5 · P4_21 J3.3 | | |
| 3.3 V | J3.4, J3.8, J4.1 | | ladder, pots, OLED, MPR121, SD, encoder |
| 5 V | J3.10 | | DAC VIN, PAM8403 |
| GND | J2.14, J3.12, J3.14, J4.3 | | |
Buttons/encoder: switch to GND, internal pull-up, no external resistors.

## Keyboard — 13-key resistor ladder
- 13× 1 kΩ 1 % in series, top → 3.3 V, bottom → GND. Key k taps the node at
  k/13 × 3.3 V (K1 = C4 = 0.25 V … K13 = C5 = 3.30 V). Stylus → J2.1, 100 kΩ to GND
  so "no key" reads 0 V. Ladder and ADC reference share the 3.3 V rail (ratiometric).
- Keys: copper-clad FR4, cut to template; check every key isolated by continuity.
- One stylus + one ADC = one key at a time: the stylus voice is monophonic by physics.

## Audio chain
- MCU SAI1 (I2S master) → UDA1334A (CJMCU-1334): VIN 5 V, leave SF0/SF1/MUTE/PLL/
  DEEM open (defaults = I2S, unmuted, PLL locks to WSEL so no MCLK needed). 3V0 is an
  output — don't feed it.
- Headphones: DAC's own 3.5 mm jack first; loudness to be tested (see DECISIONS).
- Speaker: DAC line out → PAM8403 (5 V, one channel, 10 kΩ pull-down on input) → 8 Ω.
  **PAM8403 is bridge-tied: speaker goes between OUT+ and OUT−, never to GND.**
- Volume is software (pot → ADC → scale samples).

## Power / grounding
- Power bank → USB-C J10 (powers + flashes). 5 V rail (J3.10) → DAC VIN + PAM8403.
  3.3 V rail → all logic, ladder, pots. **MPR121 = 3.3 V only (max 3.6 V).**
- Star ground: amp ground and DAC analog ground meet at one point only.

## Build order (each stage tested before the next)
v0 ✅ · **v1a** ladder → ADC → note (re-test) · v1b OLED · v1c tone via I2S → DAC →
headphones · v1d octave buttons + RGB LED · v2 samples + volume/tuning pots · v3
cutoff/LFO pots, encoder menu, presets · v4 soundpad, record/loop, speaker stage,
perfboard, enclosure · stretch: USB mass storage, USB-MIDI. Exact scope per stage
follows the answers in `docs/DECISIONS.md`.

## Firmware shape
- Setup once, then `while (1) { read inputs → decide → update outputs }`.
- Audio is the exception: the DAC needs a sample every 1/fs seconds without fail,
  so samples are produced in the SAI interrupt, never in the main loop.
- I2S format mismatch is the classic "no sound" trap (I2S, 16-bit, stereo).
- Buttons and keys need debouncing.

## Design for reuse
- Socket everything: female headers on perfboard; board + modules plug in.
- Wear order: stylus cable first (stranded silicone + strain relief + removable
  3.5 mm plug), then stressed joints (hot glue), then key tarnish (polish/lacquer).

## Cautions
1. MPR121 3.3 V only. 2. Never use P3_12, P4_15/16, P0_16/17, P0_25, P5_7 or the
LED pins as GPIO. 3. FC3 SPI is shared with the MCU-Link bridge — if SD is flaky,
that's the first suspect. 4. Star ground. 5. Feed a PC line-in, not mic-in.
6. Breadboard hisses; perfboard is quieter. 7. Never ground a PAM8403 output.

## Files
`docs/DECISIONS.md` open choices · `docs/LEARNING.md` concepts · `docs/MCXN236-
Stylophone-Wiring-Plan.md` · `docs/FRDM-MCXN236-Header-Pinout.md` ·
`docs/MCXN236-Stylophone-BOM.csv` · `nxp-docs/` board manuals + design files.
Missing (referenced earlier, never committed): Electronics-Primer.md (replaced by
LEARNING.md) and Stylophone-Key-Template.pdf.
