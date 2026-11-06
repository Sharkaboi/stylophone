# Decisions

Settled choices, each with its reason. To change one, edit it here and in `CLAUDE.md`.
Things that need your hands are at the bottom.

## Scope
- **In:** 13-key stylus keyboard, synth tones, microSD samples (an extra), OLED with
  menu + GIF screensaver, 4 knobs, encoder, 5 buttons, speaker + headphones.
- **Out:** soundpad (MPR121), record/loop, presets. Aux-in stays optional hardware (v4).
- **Reference instrument:** the original Stylophone has a buzzy square-wave tone, a
  tuning knob, a vibrato switch and a power switch, nothing else. The modern GEN X-1 adds
  sub-oscillator, filter, envelope, LFO and delay. We sit between the two.

## Software

**D1 Language: C on the board, nothing on a PC.** The gadget is standalone: power it and
play. A PC is only used to write and flash the firmware, and to design the printed parts.
No Python tools are planned.

**D2 Samples: microSD, optional.** No card, no samples on it, or the card pulled out
while playing → synth tones, automatically.
```
SD card (FAT32)
└── STYLO/
    ├── SAMPLES/
    │   └── <bank name>/      one folder = one bank; the Sound button cycles banks
    │       ├── 01.WAV … 13.WAV   one clip per key, each at its own speed (soundboard)
    │       └── any.WAV           or a single file, pitched across the 13 keys
    └── SCREENSAVER/
        └── *.GIF             looped, shrunk to 128×64 black & white
```
- WAV: PCM 8/16-bit, mono or stereo, any sample rate, any length. Clips are streamed from
  the card rather than loaded into RAM, so length is unlimited.
- The SD-over-SPI driver isn't in the installed SDK: about 150 lines to write, or fetch
  NXP's `sdmmc` component. Decided at v3.

**D3 Sample rate: 32 kHz, 16-bit.** The original is analog, so it has no sample rate to
copy. 32 kHz reproduces up to 16 kHz, beyond what the speaker or most ears use. WAVs at
44.1/48 kHz are converted while being pitched, which happens anyway.

**D4 Feeding the DAC: render in the SAI interrupt, no DMA.** Measured budget:

| Load | Cost |
|---|---|
| CPU time per sample at 32 kHz | 4,687 cycles available |
| Synth: 2 voices + filter + vibrato + echo (hardware float) | ~150 cycles ≈ 3 % |
| SAI interrupt overhead (8,000 per second, 4 frames each) | ~1 % |
| SD streaming, 64–400 KB/s over SPI (~1.5 MB/s) | ≤ ~30 % of the SPI bus, main loop |
| OLED frame, 1 KB over I2C at 400 kHz | ~23 ms → GIF at 15 fps uses ~35 % of the bus |

Rule: the main loop must never stall longer than the audio buffer (~64 ms), so the OLED
is sent in chunks.

**D5 Voices: 2 for the stylus.** The note being touched, plus the previous one fading
out, so release and echo tails overlap. The stylus can touch one key at a time.

**D6 Synth engine:**
- Waveforms: square (classic, default), saw, triangle, sine.
- Sub-oscillator one octave down.
- Attack/release envelope.
- Low-pass filter: cutoff on a knob, resonance in the menu.
- Vibrato: depth on a knob, speed in the menu.
- Echo: time and mix in the menu.

**D7 Tuning knob = the original's tuning knob.** Shifts every note by up to ±1 semitone
to match another instrument or a song. Centre = standard pitch (A4 = 440 Hz).

**D8 Record/loop: dropped.** Record outside the gadget and put clips on the SD card.

**D9 Presets: dropped.** The original has none. Menu settings reset at power-on; saving
them to the SD card can come later.

**D10 Soundpad: dropped.** The MPR121 is spare for a future project.

**D11 Controls:**

| Control | Does |
|---|---|
| Volume knob | Loudness. Firmware caps the maximum to protect IEMs |
| Tuning knob | ±1 semitone fine tune |
| Cutoff knob | Brightness (low-pass filter) |
| Vibrato knob | Vibrato depth; fully down = off |
| Octave − / + | Shift all keys an octave, ±2 octaves |
| Mode | Synth tones ↔ SD samples |
| Sound | Next waveform (tones) or next bank (samples) |
| Screen | Screensaver on/off |
| Encoder | Turn = choose setting · push = edit · turn = change |
| Toggle switch | Speaker on/off (hardware) |

**D12 RGB LED: 7 on/off colours, one per note letter;** sharps share their letter's colour.

**D13 OLED: our own small driver and drawing code.** Text, lines and bitmaps, so anything
can be drawn. Screensaver: GIFs from the SD card, decoded on the board with a small
open-source decoder (AnimatedGIF, Apache-2.0) kept in `firmware/lib/`. Without a card it
falls back to a built-in screensaver: a live oscilloscope of the sound. The Screen button
toggles it. Otherwise the display shows the note and the menu.

**D14 Code layout:** one firmware project, one folder per feature.

## Hardware

**D15 Toggle = speaker on/off,** in series with the speaker wire. The amp stays
powered, because an unpowered amp fed a signal can distort the headphones. An *LFO* (low-frequency
oscillator) is a slow wave, about 5–7 times a second, that wobbles the pitch: that's
vibrato. The vibrato knob sets how much wobble, and fully down is off. It applies to
tones and samples alike, so no switch is needed.

**D16 Headphones: the DAC's own jack.** It is wired straight to the DAC outputs, so
there's no place for the 220 Ω; IEMs are protected by the firmware volume cap. Test
loudness at v1c. If too quiet, add a real headphone amp module later.
**Never drive headphones from a PAM8403.**
- Its two outputs per channel are both driven (bridge-tied), and a headphone jack joins
  L and R at one shared sleeve, which shorts them.
- Each output sits at 2.5 V DC and switches at ~250 kHz. That DC would flow through the
  earpieces.

**D17 Speaker mute: manual,** with the toggle. The DAC jack's switch contact could
auto-mute later.

**D18 Keys + enclosure in OpenSCAD** (a programming language for 3D models). The
nickel strip already comes at key width (8 mm), so no paper template is needed: cut
13 × 50 mm pieces and drop them into printed pockets.

## Repo

**D19** Purge `nxp-docs/` from history, force-push, ignore it, and link NXP's pages.
**D20** Ignore `firmware/mcux_include.json` and `firmware/.vscode/mcuxpresso-tools.json`
(machine paths). The README documents how to recreate them.
**D21** Rewrite history without the Claude co-author line, and force-push.

## Needs your hands

| # | Check | How |
|---|---|---|
| H1 | Old PAM8403 alive? | Unpowered, Ω between its 5 V and GND pins: ~0 Ω and staying there = dead; starting low and climbing = fine (capacitors charging) |
| H2 | Nickel strip really nickel? | Cut a piece, leave the fresh edge in salt water overnight: rust = steel core, return it |
| H3 | FRDM mounting holes | Test-fit an M3 screw (likely 3.2 mm holes) |
| H4 | New OLED pin order | Read the silkscreen before wiring: `VCC GND SCL SDA` or `GND VCC SCL SDA` |
| H5 | Soldering iron | Was the Tomson 90 W digital bought? The old 25 W kit iron can't do the nickel keys |
| H6 | ADC table conditions | Was the 100 kΩ pull-down in place when the 13 ADC values were recorded? |
| H7 | Stylus tip | Do you have the brass screw/rod yet? |
| H8 | Power switch | None planned: power = power bank button or unplug. OK? |
