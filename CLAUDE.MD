# CLAUDE.md — FRDM-MCXN236 Stylophone/Soundpad build

Drop this in the repo root. It is the full project context. Read it fully before helping.

## Who I am
Developer, strong coding, **near-zero electronics experience**. Hand-hold on hardware.
Explain the *why*, keep responses tight, no fluff. Windows (VS Code). One board, free.

## What we're building
A 13-key **sample-playing stylophone + soundpad** on the FRDM-MCXN236:
stylus keyboard + capacitive soundpad, OLED screen, synth controls (volume/tuning/
filter/LFO via knobs, menu via encoder), line-out + speaker + headphones, samples
from microSD, record/loop. Out-features a real Stylophone; boutique-DIY finish target.

## STATUS: v0 DONE ✅
- Toolchain works. hello_world **builds, flashes, and prints** over serial.
- Confirmed output on **COM3 @ 115200**: "MCUX SDK version: 2026.06.00 / hello world."
- Two-way serial confirmed (send + receive).
- All parts sourced, received, and physically confirmed. Board survived shipping.
- **Next action: v1a** — build the resistor-ladder keyboard on the breadboard, read it
  on the ADC, print key values to serial.

## Working toolchain (don't re-derive — this is the proven setup)
- **VS Code + "MCUXpresso for VS Code" extension** (NOT the standalone IDE).
- MCUXpresso Installer components installed: **MCUXpresso SDK Developer, Arm GNU
  Toolchain 14.2.1, LinkServer, MCUXpresso Config Tools, cmake, ninja, git.**
- Uses the **MCUXpresso SDK (west-based) flow**, board id **`frdmmcxn236`**.
- Python 3.12.1; extension made a venv at `~/.mcuxpressotools/.mcux-venv-3.12`.
- **Gotchas already solved:**
  - Disabled Windows Store `python.exe`/`python3.exe` execution aliases (they caused
    exit code 9009). Real Python must win on PATH.
  - Needed **Ninja** on PATH and **CMake ≥ 3.30** (an old 3.28.1 warned/failed builds).
  - Dev tools + project live on the **Z: drive** (user keeps C: for Windows only).
    Keep one *complete, matched* toolchain; don't mix a half-manual Z: toolchain with
    the installer's — that caused "Ninja not found / CMake too old".
- **Flash + serial** come through **J10 (MCU-Link USB-C)**. J11 is the direct MCU USB
  (power only for our purposes). **JP6 open** = VCOM serial enabled (default).
- **To see startup prints:** open the serial monitor, then **press SW1 (reset)** — the
  greeting prints once at boot; reset replays it. (Normal; the guide's debugger-launch
  hides this because it halts at boot before printing.)
- Debug primitive: **printf to COM3** is our scope. Print ADC values, key numbers, etc.

## Hardware facts (FRDM-MCXN236, Rev C, MCXN236VDF, Cortex-M33 @150MHz, 1MB flash)
- Onboard: 8MB QSPI flash (W25Q64), accelerometer (unused), ambient light sensor
  (unused), digital mic MICFIL (unused), RGB LED, 3 buttons (SW1 reset, SW2=P0_20,
  SW3=P0_6/ISP).
- **RGB LED pins: R=P4_18, G=P4_19, B=P4_17** — occupied if we use the LED (we do, for
  per-note colour). So **do NOT reuse P4_17/18/19 as GPIO.**
- **Audio codec DA7212 + onboard J14 headphone + J21 aux jacks are DNP (not fitted)** →
  onboard audio path is dead → **external UDA1334A DAC is required** (correct choice).

## VERIFIED PIN MAP (from UM12041 header tables — starting map; finalize pads in Config Tools)
Buses fit with no hard conflict. ~23 signals on Arduino headers J1/J2/J4.

**I2S / SAI1 (DAC) — J1:**
- BCLK = P3_16 (J1.1) · WSEL = P3_17 (J1.11) · DIN = P2_8 (J1.5) · MCLK = P3_6 (J1.7, leave unused)

**I2C (FC5) — J2:** SDA = P1_16 (J2.18) · SCL = P1_17 (J2.20). OLED 0x3C, MPR121 0x5A.

**SPI (FC3) — J2:** CS = P1_3 (J2.6) · MOSI = P1_0 (J2.8) · MISO = P1_2 (J2.10) · SCK = P1_1 (J2.12).
- CAUTION: FC3 shared with MCU-Link USB-SPI bridge via 0Ω R153–R156 (populated).
  Works for SD as-is; if flaky, isolate those resistors.

**ADC (5 ch: keyboard + 4 pots):** among P0_25(J2.11), P0_26(J2.7), P0_27(J2.5),
  P0_28(J2.3), P0_29(J2.1). (These are light-sensor/motor-feedback pins, all free.)

**GPIO pool (need 9: encoder×3 + buttons×5 + toggle×1)** — clean candidates:
  P2_0(J1.6), P2_7(J1.12), P0_22(J1.16), P0_21(J1.10), P0_23(J2.2), P0_24(J2.17),
  P4_6(J4.2), P4_12(J4.10), P4_13(J4.12). Internal pull-ups → no external resistors.
- **AVOID as GPIO:** P3_12 (QSPI flash D3), P3_17 (SAI WSEL, used), P4_17/18/19 (RGB LED).
- **Reserved (never touch):** SWD/debug, ISP/reset/wake.

## Keyboard — 13-key resistor ladder (KEY CUSTOM PART)
- 13× **1kΩ 1% metal film** (Yageo MFR-12, have 20) in series, top→3.3V, bottom→GND.
- Each junction taps one key pad. Stylus → one ADC pin, with **100kΩ pull-down** to GND
  (no-key reads 0V). Power ladder from the **same 3.3V as the ADC reference** (ratiometric).
- Keys land ~0.25V apart (~315 ADC counts on 12-bit): K1=C4=0.25V … K13=C5=3.30V, none=0V.
- Keys = **copper-clad FR4**, hand-cut with the 1:1 printed template. Confirm each key
  isolated with multimeter continuity before wiring.

## Audio chain
- MCU I2S → **UDA1334A DAC** (WCMCU-1334; VIN 3–5V, feed 5V; leave SF0/SF1/MUTE/PLL/DEEM
  default; 3V0 is an output, don't feed).
- DAC Lout/Rout → **220Ω series** (IEM level/protect) → headphone out. **AGND** = analog gnd.
- DAC has an **onboard switched 3.5mm jack** — usable as THE headphone jack (mount to
  enclosure edge; its switch/Func pin can auto-mute the speaker). Simplest option.
- Loose panel jacks are **plain 3-pin** (no switch) → use for **aux-in + stylus socket**;
  speaker mute via the **toggle** (manual), or via the DAC jack's switch pin.
- DAC line-out → **PAM8403** (VCC 5V, one channel, +10k pulldown on input) → **8Ω speaker**.
- **Volume = software** (volume pot → ADC → scale samples). No analog volume.
- Aux-in (v4, optional): 10k passive mix into the line node.

## Controls / display
- I2C shared bus: OLED (0x3C) + MPR121 (0x5A, **3.3V ONLY, max 3.6V**). Both have onboard
  pull-ups; if bus flaky, cut MPR121's pull-up jumpers.
- SPI: Mini SD module (3.3V-native), FatFs, cards FAT32.
- 4 pots (B10K linear, wire across 3.3V) on ADC: volume/tuning/cutoff/LFO-depth.
- KY-040 encoder (A/B/SW, 3.3V) + 5 buttons + optional vibrato toggle on GPIO.

## Power / grounding
- 5V via USB (power bank). 5V rail → DAC VIN + PAM8403. 3.3V rail (onboard reg) → all
  logic + ladder + pots + ADC ref. **MPR121 never 5V.**
- **Star ground:** amp/speaker ground returns separately from DAC AGND, joined at one point.

## Build order (each stage tested before the next)
- **v0 ✅** hello_world builds/flashes/prints.
- **v1a (NEXT)** resistor ladder → read on ADC → print key# to serial. First custom code.
- **v1b** add OLED, show ADC value / note.
- **v1c** tone: key → note → I2S → DAC → headphones (map key→freq, equal temperament).
- **v1d** octave buttons + RGB LED per note + transpose.
- **v2** SD + FatFs → WAV playback pitched across keys; volume + tuning pots.
- **v3** cutoff + LFO pots, vibrato toggle, menu encoder (waveform/sub/envelope/delay),
  preset save/load.
- **v4** MPR121 soundpad + software mixing (voice pool + stealing), record/loop, audio
  output stage (jacks+amp+speaker+aux), perfboard, 3D-printed enclosure.
- **stretch (software only):** USB mass-storage (drag WAVs on), USB-MIDI.

## Firmware shape (the pattern)
Embedded = setup once, then `while(1)` { read inputs → decide → drive outputs → tiny wait }.
- v1 tone = PWM/hardware-timer, simple loop is fine.
- **v2+ audio playback must be fed to the DAC on a timer interrupt / DMA at a fixed rate
  (~22050 Hz)** — cannot live in the slow main loop or it glitches. This is where
  interrupts/DMA enter. Expect I2S format matching to be the classic "no sound" trap.
- Buttons need debouncing.

## Design-for-reuse (important to the user)
- **Socket everything; solder nothing to the modules or the FRDM board.** Solder female
  headers to the perfboard; plug the board + DAC + MPR121 + SD + OLED into them. Later the
  board pops out for reuse with **zero desoldering**. Only cheap passives/jacks/keys are
  permanent.
- **First to fail:** the stylus cable (flex fatigue) — stranded silicone + strain relief +
  removable 3.5mm plug. Then stressed solder joints (hot-glue reinforce), then key tarnish
  (re-polish/lacquer). Expensive modules fail last.

## Cautions recap
1. MPR121 = 3.3V only. 2. Don't GPIO P3_12/P3_17/P4_17-19. 3. FC3 SPI shared w/ MCU-Link
bridge. 4. Star ground for clean audio. 5. Feed a PC **line-in**, not mic-in. 6. Breadboard
will hiss; improves on perfboard. 7. Finalize exact pads in **MCUXpresso Config Tools**.

## Companion files (attach if referenced)
BOM CSV · Wiring-Plan.md · Electronics-Primer.md · Stylophone-Key-Template.pdf · UM12041 (board UM).

## Immediate next step
Guide v1a: (1) breadboard the 13× 1k ladder + 100k pulldown, verify the 0.25V steps with a
multimeter, (2) modify hello_world to init LPADC on one channel and printf the raw value +
decoded key number to COM3, (3) confirm each key reads its expected band.