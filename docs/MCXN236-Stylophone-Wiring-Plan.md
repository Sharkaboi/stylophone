# FRDM-MCXN236 Stylophone — Master Wiring & Build Plan

Complete electrical plan for the 13-key sample stylophone/soundpad. Four
subsystems: power/grounding, keyboard ladder, audio chain, controls/display.
Pair this with the BOM. Assign the concrete board pads in **MCUXpresso Config
Tools** (`frdmmcxn236`) — each peripheral function is only available on certain
pads, so the names below are the logical plan, not fixed pin numbers.

---

## Signal / pin budget

| Bus | Signals | Goes to |
|---|---|---|
| ADC (LPADC) | 5 | key ladder (1) + volume/tuning/cutoff/LFO pots (4) |
| I²C (LPI2C) | 2 | OLED (0x3C) + MPR121 (0x5A), shared |
| SPI (LPSPI) | 4 | SD card (SCK, MOSI, MISO, CS) |
| I²S (SAI) | 3 | UDA1334A DAC (BCLK, WSEL, DIN) |
| GPIO | 9 | encoder (A,B,SW) + 5 buttons + vibrato toggle |
| Power | — | 3.3 V, 5 V (VBUS), GND |

≈ 23 signals — fits the Arduino/FRDM header with room to spare. Reserved: do not
reuse the SWD/debug or ISP/reset/wake pins.

---

## 1. Power & grounding

**Rails**
- Source: power bank → USB-C (MCU-Link port). This port both powers the board and
  flashes it. On/off = power bank button, or a slide switch in the 5 V feed.
- **5 V rail (VBUS):** PAM8403 amp VCC, UDA1334A VIN. (Higher current + the DAC.)
- **3.3 V rail (on-board regulator):** OLED, MPR121, SD card, KY-040 encoder, the
  4 pots (top of each), and the top of the key ladder. This 3.3 V is **also the
  ADC reference** — powering the ladder and pots from it makes them ratiometric,
  so readings don't drift as the battery sags.

**MPR121 is 3.3 V only (max 3.6 V) — never wire it to 5 V.**

**Grounding — star topology.** Run these ground groups back to one common star
point rather than daisy-chaining:
- Analog/audio ground: DAC AGND, the 220 Ω / aux mix returns, jack sleeve.
- Amp/speaker ground: PAM8403 GND + speaker return (this is the noisy, high-current one).
- Digital ground: MCU GND, SD, I²C devices, logic.
Keep the amp/speaker ground path separate from the DAC's analog ground until the
star — that's what stops speaker current injecting hiss into the line-out.

---

## 2. Keyboard — 13-key resistor ladder

**Chain:** 13× 1 kΩ (1% metal film) in series, top → 3.3 V, bottom → GND.
Continuous current ~0.25 mA (negligible).

**Taps:** the junction between each resistor pair goes to one key pad. Top rail
(3.3 V) = Key 13 (C5); junction just above GND = Key 1 (C4). Keys land at evenly
spaced voltages ~0.25 V apart.

**Stylus:** the common pickup — wires (via the removable PJ311 stylus jack) to
**one ADC pin**. A **100 kΩ pull-down** from that ADC pin to GND makes "no key" read 0 V.

| Key | Note | Voltage | ADC (12-bit) |
|---|---|---|---|
| 13 | C5 | 3.30 | ~4095 |
| 12 | B4 | 3.05 | ~3780 |
| 11 | A#4 | 2.79 | ~3465 |
| 10 | A4 | 2.54 | ~3150 |
| 9 | G#4 | 2.28 | ~2835 |
| 8 | G4 | 2.03 | ~2520 |
| 7 | F#4 | 1.77 | ~2205 |
| 6 | F4 | 1.52 | ~1890 |
| 5 | E4 | 1.27 | ~1575 |
| 4 | D#4 | 1.02 | ~1260 |
| 3 | D4 | 0.76 | ~945 |
| 2 | C#4 | 0.51 | ~630 |
| 1 | C4 | 0.25 | ~315 |
| — | none | 0 | ~0 |

Bands sit ~315 counts apart — hugely reliable. Firmware maps each band to a note;
transpose/octave just shift the mapping.

**Pins:** 3.3 V, GND, 1 ADC channel.

---

## 3. Audio chain

**MCU I²S → DAC.** MCU SAI BCLK → DAC **BCLK**, LRCLK → **WSEL**, data → **DIN**.
DAC **VIN → 5 V** (board accepts 3–5 V), **GND**, **AGND** for the analog return.
Leave the config header (SCLK, MUTE, PLL, SF0, SF1, DEEM) unconnected — on-board
defaults give standard I²S, un-muted. `3V0` is a regulator *output* — don't feed it.

**Line out → headphone jack (auto-mute).**
- DAC **Lout → 220 Ω → PJ311 tip**; **Rout → 220 Ω → PJ311 ring**; **AGND → sleeve**.
  (220 Ω tames level and protects IEMs.)
- The PJ311 has a normally-closed **switch contact** on the tip. Route the DAC
  signal to the amp *through* that contact: no plug → contact closed → amp fed →
  speaker plays; plug inserted → contact opens → amp cut → **speaker auto-mutes**,
  headphones play. (Confirm the switch pin with the multimeter: with no plug, tip
  and tip-switch beep together; inserting a plug breaks the beep.)

**Amp → speaker.**
- PAM8403 **VCC → 5 V**, **GND → (amp) star ground**. One channel used (mono speaker).
- Amp input ← the switched line node. Add a **10 kΩ pulldown** from amp input to
  ground so it sits quiet while muted (no buzz/pop).
- Amp output → **speaker (8 Ω)**.

**Aux-in (optional, v4).** Aux jack tip/ring → **10 kΩ each** into the line node
(passive sum), sleeve → ground. External audio then plays through speaker and
headphones alongside the instrument.

**Volume** is software: the volume pot → ADC → scales samples. No analog volume
in the path.

---

## 4. Controls & display

**I²C (shared bus):** SDA, SCL to both OLED (0x3C) and MPR121 (0x5A). Both modules
have on-board pull-ups; if the bus is flaky with several devices, cut the MPR121's
pull-up jumpers. Power both from 3.3 V. MPR121 IRQ pin optional (poll instead); ADD
left as-is for 0x5A.

**SPI:** SD card — SCK, MOSI, MISO, CS. Power from 3.3 V (module is 3.3 V-native).
Use a FatFs filesystem; cards formatted FAT32.

**ADC:** 4 pots — each pot's outer legs to 3.3 V and GND, wiper to its own ADC
channel (volume, tuning, filter cutoff, LFO depth). Plus the key ladder on its ADC
channel. Wire pots across 3.3 V (their 5 V rating is just a max).

**GPIO (internal pull-ups, no external resistors):**
- KY-040 encoder: A (CLK), B (DT), SW (push) — 3 pins. Use interrupt-capable pins
  or poll fast. Power 3.3 V.
- 5 buttons: octave −, octave +, mode, preset, record/play — each button to a GPIO
  and to GND.
- Vibrato toggle: 1 GPIO to GND (optional).

**RGB LED:** on-board, no wiring — colour per note/pad.

---

## 5. Anti-noise checklist (for clean line-out)

1. Power the DAC from a steady rail; keep its GND short and direct to the star.
2. Keep I²S and analog audio runs short; route them away from the SD/SPI and
   switching lines. The soldered perfboard build is quieter than the breadboard.
3. Amp/speaker ground returns to the star separately from the DAC analog ground.
4. Feed a PC **line-in**, not mic-in (use a USB sound card if the PC only has mic).

---

## 6. Build order

**v0 — toolchain:** flash `hello_world`, confirm serial + RGB LED.

**v1 — keyboard + tone (breadboard):** build the ladder, read it on ADC, generate
the note via I²S → DAC → headphones (DAC is in from v1, so monitor on headphones —
no buzzer). Show note on OLED + LED. Octave ± buttons. Add transpose (trivial).
*Needs:* DAC, OLED, encoder, buttons, ladder resistors + 100 k, breadboard, wires,
multimeter.

**v2 — samples:** add SD on SPI + FatFs; play WAV pitched across keys; wire the
volume + tuning pots. Preset toggles synth vs sample.
*Needs:* SD module (cards owned).

**v3 — full panel:** cutoff + LFO-depth pots, vibrato toggle, menu encoder
(waveform, sub-osc, resonance, envelope, delay), preset save/load. Tilt/light
features are dropped.
*Needs:* remaining pots, toggle.

**v4 — soundpad + outputs + enclosure:** MPR121 pads + software mixing
(voice pool + stealing); record/replay + looping; headphone jack (switched
auto-mute) + amp + speaker; aux-in. Move to perfboard (socket modules on female
headers), 3D-printed shell, star grounding. Copper-clad key face cut and mounted.
*Needs:* MPR121, PAM8403, speaker, 3× PJ311, perfboard, headers, standoffs.

**Stretch (software only):** USB mass-storage (drag WAVs on), USB-MIDI
(driverless controller for Mac/PC).

---

## 7. Header pin summary

| Signal | Type | Subsystem |
|---|---|---|
| 3.3 V | power | ladder top, pots, OLED, MPR121, SD, encoder, ADC ref |
| 5 V (VBUS) | power | DAC VIN, PAM8403 |
| GND | power | star ground |
| ADC ×5 | analog in | key ladder + 4 pots |
| SDA, SCL | I²C | OLED, MPR121 |
| SCK, MOSI, MISO, CS | SPI | SD card |
| BCLK, WSEL, DIN | I²S | DAC |
| GPIO ×9 | digital | encoder ×3, buttons ×5, toggle ×1 |

Assign the concrete pads in MCUXpresso Config Tools before wiring the permanent build.
