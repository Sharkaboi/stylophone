# Decisions

Every open choice in one place. ★ = recommendation, with the reason.
Answer like "accept all ★" or "D3 = b, rest ★". Settled answers move into `CLAUDE.md`.

## Physical checks (only you can do these)

| # | Check | How | Why it matters |
|---|---|---|---|
| P1 | Parts on hand | Fill the Status column below | Decides what we can build next |
| P2 | Ladder voltages + pull-down | Multimeter DC-V, black on GND, red on each tap: expect k × 0.254 V (K1 0.25 … K13 3.30). Then Ω mode on the pull-down alone (out of circuit): expect ~100 kΩ (brown-black-yellow) | Proves the hardware before blaming code; explains the mid-ladder sag |
| P3 | PJ311 jack pin count | 3 pins = plain, 5 pins = has switch contacts | Decides speaker auto-mute (D9) |
| P4 | DAC module's own 3.5 mm jack | Is there one on the CJMCU-1334 board? | Simplest headphone output |
| P5 | OLED pin order + address | Read silkscreen: `VCC GND SCL SDA` or `GND VCC …`; address jumper 0x3C/0x78 | Swapped VCC/GND kills it instantly |
| P6 | Module header pins | Do DAC / OLED / MPR121 / encoder come with loose (unsoldered) pins? | Loose pins don't make contact on a breadboard |
| P7 | Soldering kit | Iron, solder, flux, wick? Not in the BOM | Needed for module pins, stylus, perfboard |

### P1 — parts status (fill in: ✅ tested · 📦 have, untested · ❌ missing)

| Part | Status |
|---|---|
| FRDM-MCXN236 + USB-C cable | ✅ |
| UDA1334A DAC (CJMCU-1334) | |
| PAM8403 amp | |
| 8 Ω 1 W speaker | |
| PJ311 3.5 mm jacks ×3 | |
| SSD1306 0.96" OLED (I2C) | |
| MPR121 touch board | |
| Mini micro-SD module + cards | |
| Tactile buttons ×10 | |
| KY-040 encoder | |
| 10 kΩ pots ×4 + knobs | |
| SPDT toggle | |
| 1 kΩ 1 % ×13 ladder (on breadboard?) | |
| 100 kΩ + 220 Ω resistors | |
| FR4 copper clad (keys cut?) | |
| Teflon 20 AWG wire + brass stylus tip | |
| Breadboard + jumper wires | |
| Perfboard ×2, headers, standoffs | |
| Multimeter | |
| Power bank | |
| 3D printer | ✅ |
| Soldering kit | |

## Software

**D1 — Language.** ★ **C on the board, Python on the PC.** The audio interrupt must
produce a sample every 31 µs and never miss. MicroPython runs tens of times slower than C,
and this chip has no mainstream MicroPython/Arduino support. Kotlin has no bare-metal
target. Python fits the PC side well: sample converter, key-template generator,
serial plotter.

**D2 — Where samples live.**
- a) ★ **Built into flash.** A Python tool turns any WAV into a C array and you
  re-flash to change kits. About 900 KB free ≈ 14 s of audio at 32 kHz. No SD wiring,
  no SPI or filesystem code, and it avoids the FC3/MCU-Link conflict.
- b) microSD: load the chosen kit into RAM at boot. Swap sounds without a PC, but
  RAM limits you to ~2 s. Needs SPI + FatFs + an SD-over-SPI driver that is *not*
  in the installed SDK (about 150 lines to write or fetch).
- c) microSD copied into the 8 MB QSPI flash: most capacity, most code.

Option a) first; b) can be added later as its own stage without changing anything else.

**D3 — Sample rate.** ★ **32 kHz, 16-bit.** Sound up to 16 kHz (half the rate) is
plenty for this instrument. It uses a third less RAM and CPU than 48 kHz, and the
DAC's PLL locks to anything from 16 to 100 kHz. Alternative: 48 kHz.

**D4 — Feeding the DAC.** ★ **Render samples inside the SAI FIFO interrupt.** It is the
shortest code, and 150 MHz gives ~4,700 CPU cycles per sample. DMA is the
alternative: more setup code, needed only if the CPU gets tight.

**D5 — Voices.** ★ **1 stylus voice + 7 pad voices; a new pad hit takes the oldest.**
One stylus can only touch one key, so the keyboard is monophonic by physics.

**D6 — Synth engine (v3).** ★ Oscillator: sine / triangle / saw / square / sample,
plus a sub-oscillator one octave down → attack–release envelope (2 menu settings
instead of ADSR's 4) → 2-pole state-variable low-pass filter (cutoff pot, resonance in menu)
→ echo delay (time + mix in menu) → volume pot. The LFO bends pitch for vibrato
(depth pot, rate in menu). Each block is roughly 5 lines of maths.

**D7 — Tuning pot.** ★ **Fine tune ±1 semitone** (matches other instruments by ear).
Octave buttons and a menu transpose cover big jumps. Alternative: ±12 semitones.

**D8 — Record / loop (v4).** ★ **Record note events** (time, note, on/off), not audio.
A minute of playing takes about 2 KB, overdub is free, and playback goes through the
current sound. Recording audio would fill RAM in ~3 s.

**D9 — Presets.** ★ **8 slots in the last sector of internal flash.** Uses the SDK
flash API with no extra hardware. Alternative: a file on SD (only if D2 = b/c).

**D10 — Soundpad (v4).** ★ **8 pads that play one-shot drum samples** (kick, snare,
hat, clap, …), made from copper tape or FR4 squares under a thin printed lid. The
MPR121's other 4 inputs stay spare.

**D11 — Controls.** ★ Buttons: octave −, octave +, mode (synth ↔ sampler), preset
(tap = next, hold = save), record (tap = record/stop, hold = clear). Encoder: turn to
pick a parameter, push to edit it, turn to change the value.

**D12 — RGB LED.** ★ **7 on/off colours, one per note name (C, D, E …), shared by
sharps.** No timers needed. Alternative: PWM for 12 distinct hues.

**D13 — OLED.** ★ **Our own tiny SSD1306 driver** (init list + 1 KB screen buffer +
5×7 font, ~80 lines). The SDK's LVGL is a full GUI framework and overkill here.

**D14 — Code layout.** ★ One firmware project with one file per job: `main.c`
(loop), `keys.c`, `knobs.c`, `buttons.c`, `audio.c` (SAI + interrupt), `synth.c`,
`oled.c`, `ui.c`, `pads.c`, `store.c`. PC tools go in `tools/`.

## Hardware

**D15 — Toggle switch.** ★ **Speaker on/off**, wired into the PAM8403's 5 V feed: it
silences the speaker and kills amp hiss on headphones. Vibrato needs no switch,
because LFO depth at zero means off. Saves a GPIO. Alternative: vibrato on/off.

**D16 — Headphone output.** ★ **Use the DAC's own jack; try it without the 220 Ω
first.** The UDA1334A is a line output, designed for loads of thousands of ohms.
Headphones are 16–32 Ω, so it may be quiet, and a 220 Ω series resistor makes it
~8× quieter still with 32 Ω headphones (voltage divider: 32 / (220 + 32)). Test at v1c. If it's too quiet, add a headphone amp chip (not
in the BOM).

**D17 — Speaker auto-mute.** ★ **Manual, via the D15 toggle.** Auto-mute needs a
switched jack (P3), which the BOM doubts you have.

**D18 — Key template + enclosure.** ★ **Generate both from code**: Python → 1:1 SVG
key template, OpenSCAD (a programming language for 3D models) → enclosure STL.
Version-controlled, parametric, fits how you work.

## Repo

**D19 — `nxp-docs/` (≈25 MB of NXP PDFs + design files) is already public.**
★ Stop tracking it from now on (keep local copies, link NXP's download pages in the
docs). Optional: purge it from history, which needs a force-push and can't be undone
on GitHub. NXP owns those files. Alternative: leave as is.

**D20 — Machine paths in tracked files.** `firmware/mcux_include.json` and
`.vscode/mcuxpresso-tools.json` hold `C:/Users/sarat/...` and `z:\frdm_mcxn236`. The
extension needs them. ★ Keep them tracked: low sensitivity, and the build breaks
without them. Alternative: ignore them and document the setup.

**D21 — Claude attribution in pushed history.** Commit `8d9318f` (already pushed)
ends with a `Co-Authored-By: Claude` line. ★ Leave it: removing it needs a
force-push. New commits carry no attribution.
