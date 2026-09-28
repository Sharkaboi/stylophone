# Stylophone build — handoff

Project: 13-key sample stylophone + soundpad on FRDM-MCXN236. Dev, no electronics experience. Pin map verified against UM12041.

## Status
- BOM: final, ordered/carted (~₹1,650). Not yet built.
- Pin map: verified, fits, no hard conflicts.
- Next: install MCUXpresso + Config Tools, confirm map, write v1 firmware.

## Board facts
- MCXN236, Cortex-M33 150MHz, 1MB flash + 8MB QSPI.
- Audio codec (DA7212), onboard headphone jack (J14), aux jack (J21) = all DNP / dead. External DAC required.
- Power: 5V via USB-C (J10, also flashes). Onboard 3.3V rail = ADC ref.
- RGB LED: R=P4_18, G=P4_19, B=P4_17. Buttons: SW1 reset, SW2=P0_20, SW3=P0_6.

## Verified pin map (from UM12041 header tables)
Audio I2S / SAI1 — J1:
- BCLK  = P3_16  J1.1
- WSEL  = P3_17  J1.11
- DIN   = P2_8   J1.5
- MCLK  = P3_6   J1.7 (leave unused, DAC self-clocks)

I2C (OLED 0x3C + MPR121 0x5A) — J2 / FC5:
- SDA = P1_16  J2.18
- SCL = P1_17  J2.20

SPI (SD card) — J2 / FC3:
- CS   = P1_3  J2.6
- MOSI = P1_0  J2.8
- MISO = P1_2  J2.10
- SCK  = P1_1  J2.12

ADC — J2 analog row (5 ch: keyboard + 4 pots):
- P0_25 J2.11, P0_26 J2.7, P0_27 J2.5, P0_28 J2.3, P0_29 J2.1

GPIO (encoder 3 + buttons 5 + toggle 1 = 9) — pick from clean pins:
- P2_0 (D2), P2_7 (D5), P0_22 (D7), P4_6 (A0), P4_12 (A4), P4_13 (A5), P4_17 (A3)
- avoid P3_12 (=QSPI flash), P3_17 (=SAI WSEL already used)

## Keyboard ladder (built, not on board)
- 13x 1k 1% in series, 3.3V -> GND. Each junction -> one key pad.
- Stylus -> one ADC pin, with 100k pulldown to GND.
- Power ladder from same 3.3V as ADC ref (ratiometric).
- Key voltages ~0.25V apart. K1=C4=0.25V ... K13=C5=3.30V. No-key=0V.

## Audio chain
MCU I2S -> UDA1334A (VIN 5V, 3-5V ok) -> Lout/Rout -> 220R x2 -> PJ311 switched jack -> PAM8403 (5V, +10k pulldown on input) -> speaker.
- PJ311 tip-switch break contact = auto-mute speaker when headphones in. Confirm switch pin w/ multimeter.
- Aux-in: 10k passive mix into line node (v4, optional).
- Volume = software (pot on ADC).

## Grounding
- Star ground. Amp/speaker GND return separate from DAC AGND until star point.

## Cautions (real, not blockers)
1. SPI FC3 shared w/ MCU-Link USB-SPI bridge (0R R153-156, populated). SD works as-is; isolate if flaky.
2. Keys: copper-clad FR4, hand-cut gaps. Confirm each key isolated w/ multimeter continuity before wiring.
3. I2C: OLED + MPR121 both have pullups. If bus flaky, cut MPR121 pullup jumpers.
4. MPR121 = 3.3V ONLY (max 3.6V). Never 5V.
5. I2S bring-up: SAI config must match DAC format (I2S standard, 16-bit). Classic no-sound trap.
6. Breadboard will have some audio hiss; improves on perfboard.

## Build order
- v0: hello_world, serial, blink RGB.
- v1: read ladder (ADC) -> note via I2S->DAC->headphones. OLED shows note + LED. octave buttons. transpose. (DAC in from v1, no buzzer.)
- v2: SD (SPI+FatFs) -> WAV playback pitched. volume + tuning pots.
- v3: cutoff + LFO pots, vibrato toggle, menu encoder (waveform/sub/envelope/delay), preset save/load.
- v4: MPR121 soundpad + mixing (voice pool + stealing), record/loop, headphone jack + amp + speaker + aux, perfboard + printed enclosure.
- stretch: USB mass-storage, USB-MIDI (both software, 0 pins).

## Tooling
- MCUXpresso for VS Code (or IDE) + frdmmcxn236 SDK. Import SDK example.
- LinkServer for MCU-Link. Flash + debug via J10.
- Firmware phase: move to Claude Code (check Pro includes it).

## Files to carry over
- MCXN236-Stylophone-BOM.csv
- MCXN236-Stylophone-Wiring-Plan.md
- This handoff.

## Attach in new chat
- UM12041 (board UM), QSG, this handoff, BOM, wiring plan.
