# Learning notes

The physics and ideas behind every part of this build, in the order they show up.
Each entry covers what it is, why it works, and a rule of thumb you can reuse on other
projects. New entries are added as the build introduces new ideas.

## 1. Voltage, current, resistance
Voltage is electrical pressure difference, current is flow, resistance is how much a
part restricts the flow. **V = I × R** (Ohm's law). Power turned into heat: **P = V × I**.
Example: the ladder is 13 kΩ across 3.3 V → I = 3.3 / 13,000 = 0.25 mA, P = 0.8 mW.
Rule: compute I and P for every new part; heat and battery life come straight out of it.

## 2. Voltage divider (the key ladder)
Resistors in series share the voltage in proportion to their resistance:
`Vout = Vin × R_below / (R_below + R_above)`. Thirteen equal resistors give 13 equal
steps of 3.3/13 = 0.254 V. 1 % resistors keep each step within about ±0.01 V of target.
Rule: a divider turns a position (which key) into a voltage, and an ADC turns that into
a number. Pots work the same way: a pot is a divider you slide.

## 3. Floating inputs and pull resistors
An input connected to nothing is a tiny capacitor with almost infinite resistance. It
picks up mains hum and static and reads random values. A **pull-down** (resistor to
GND) or **pull-up** (to 3.3 V) gives it a default value. The resistor must be weak
enough that a real signal overrides it: a key (≤ 3.25 kΩ) easily overrides 100 kΩ.
Buttons use the chip's internal pull-up: released = 1, pressed (shorted to GND) = 0.
Rule: every input must always be driven by something, even if only a resistor.

## 4. Loading and source resistance
Any source behaves like a perfect voltage behind a series resistance. For ladder tap k
that resistance is (k × (13 − k) / 13) kΩ: 0.9 kΩ at the ends, 3.25 kΩ in the middle.
Whatever you connect (100 kΩ pull-down, ADC input) forms a second divider with it.
A 100 kΩ load drops the middle key by 3.25 / 103.25 ≈ 3 %.
Rule: make the load at least 10× (ideally 100×) the source resistance.

## 5. ADC (analog → digital)
The ADC compares the input with a reference voltage: **N = Vin / Vref × 4095** (12-bit).
Internally it charges a tiny sampling capacitor (a few pF) through the source
resistance, which takes about 9 × R × C to settle to 12-bit accuracy. If the sample time
is too short for a high-resistance source, readings sag.
Rule: know your reference, and give high-resistance sources enough sample time.

## 6. Ratiometric measurement
If the ladder and the ADC reference come from the same 3.3 V, a sagging supply scales
both equally and N stays the same. This is why the ladder runs from the ADC's own rail,
and why the firmware must select that rail (VREFH) as the reference.

Reading the evidence: a wrong reference shifts every key by the *same factor*.
v1a's keys are right at both ends and low only in the middle. That shape follows the
source resistance from entry 4, which is a loading or settling problem, not a reference
problem. Matching the *shape* of an error to a cause is the core debugging skill.
How it resolved: a longer sample time cut the mid-ladder sag from 11 % to 3 %. The 3 %
left is exactly the 100 kΩ loading from entry 4: predicted 2136 at K7, measured 2137.

## 7. Bounce and debouncing
Metal contacts bounce, making and breaking dozens of times over 1–10 ms, and a stylus
scraping across copper does the same. Software fix: accept a new state only after it
has been stable for N readings in a row.

## 8. Quadrature (rotary) encoder
Two switches, A and B, a quarter step apart. Turning one way, A changes before B;
turning the other way, B changes first. On each change of A, B's level tells the
direction.

## 9. Digital buses
- **I2C**: 2 wires shared by many devices, each with an address (OLED 0x3C, MPR121
  0x5A). The lines are *open-drain*: devices can only pull them low, and resistors pull
  them high. So pull-ups are mandatory, and all pull-ups on the bus act in parallel
  (board 4.7 kΩ + modules ≈ 2 kΩ total, fine; below ~1 kΩ devices struggle to pull low).
- **SPI**: 4 wires, driven high and low (*push-pull*), so it's faster, with one
  chip-select wire per device.
- **I2S**: 3 wires for audio: bit clock, word select (left/right), data.
  Bit clock = sample rate × bits × channels = 32,000 × 16 × 2 = 1.024 MHz.

## 10. Sampling and Nyquist
Sound is a pressure wave. Digital audio is its value measured fs times per second. The
highest frequency it can represent is **fs / 2**. Higher ones fold back as false lower
tones (*aliasing*), which is why naive square and saw waves sound harsh on high notes.

## 11. Musical pitch
An octave doubles the frequency. Equal temperament splits it into 12 equal ratios of
2^(1/12) ≈ 1.0595: **f = 440 × 2^((n − 69) / 12)**, with n = MIDI note (A4 = 69,
C4 = 60 = 261.63 Hz). Playing a sample 1.0595× faster raises it one semitone, which is
also why pitched-up samples sound chipmunk-like.

## 12. Real-time deadlines and interrupts
At 32 kHz the DAC needs a new sample every 31.25 µs. Late means an audible click. An
*interrupt* is the hardware tapping the CPU on the shoulder: the SAI's buffer runs low,
the CPU pauses the main loop, computes samples, and returns. Slow work (screen, knobs)
stays in the main loop.

## 13. Line, headphone and speaker levels
Line out is about 1 V into ≥ 10 kΩ, a tiny current. Headphones (16–32 Ω) need much
more current for the same voltage (I = V / R). Speakers (8 Ω) need watts (P = V² / R),
hence a power amplifier.
Rule: match the output to the load's impedance, not just its voltage.

## 14. Class-D, bridge-tied amplifier (PAM8403)
It switches its outputs fully on and off ~250,000 times a second; the speaker's
inertia and inductance average that into sound, so little energy is wasted as heat.
Both speaker terminals are driven in opposite directions (*bridge-tied load*), giving 4×
the power from 5 V. Neither terminal is ground, so wiring one to GND shorts an output.
The same reason rules it out for headphones: a headphone jack joins left and right at one
shared sleeve. Each output also sits at 2.5 V DC, which would flow through the earpieces.

## 15. Grounding
Wires have resistance. Current through a shared ground wire creates a small voltage
(V = I × R) that every device on that wire sees as noise. Speaker current is large and
pulsing, so it gets its own return path to a single *star* point.

## 16. Capacitive touch (MPR121)
A pad and your finger form a capacitor (your body is a large conductor coupled to
ground). A touch adds a few pF. The MPR121 measures how long each pad takes to charge
and flags a touch when that time changes.

## 17. Flash vs RAM
Flash keeps data without power, but is erased in whole sectors and wears out after
~10,000+ writes: good for samples and presets. RAM is fast and loses everything at
power-off: good for working data.

## 18. Choosing a part
Ask, in order: does it do the job → is it voltage-compatible (3.3 V logic here) → does it
use an interface you have free (I2C / SPI / I2S / GPIO / ADC) → module or bare chip (a
module = a chip plus its support parts on a breakout) → the cheapest one that passes.
Read the datasheet, not the listing: the "OLED" bought first turned out to be a bare
panel that needs its own PCB.

## 19. Will it burn?
Compute **I = V / R**, then **P = I² × R**, and compare P with the part's rating.
Example: 10 Ω across 3.3 V draws 0.33 A and dissipates 1.1 W, which smokes a ¼ W resistor.
The ladder: 0.25 mA through 1 kΩ is 0.06 mW per resistor, so it can't heat up at all.

## 20. Tolerance: metal film vs carbon film
Metal film is ±1 %, stable with temperature, and quiet. Carbon film is ±5 %, drifts with
heat, and adds a little noise. Tolerance matters when exact values or *matching* matter
(13 equal ladder steps); it doesn't when a resistor just has to be "roughly this big"
(a 100 kΩ pull-down: anything from 47 kΩ to 220 kΩ works).

## 21. Synth building blocks, in plain words
- **Oscillator**: makes the raw tone. The waveform shape sets the character: *square* is
  buzzy and hollow (the classic Stylophone sound), *saw* is bright, *triangle* is soft,
  *sine* is pure.
- **Sub-oscillator**: a second tone one octave lower, mixed in for weight.
- **Envelope**: how a note starts and ends. *Attack* = fade-in time, *release* = fade-out
  time after the stylus lifts. A few ms of each also removes clicks.
- **Low-pass filter**: removes high frequencies, so the sound gets darker as the cutoff
  goes down. *Resonance* boosts the frequencies right at the cutoff, making it sound "wah"-like.
- **LFO and vibrato**: a slow wave (about 5–7 Hz) that nudges the pitch up and down.
  Depth = how far it nudges.
- **Echo**: the sound repeated after a delay, each repeat quieter. *Time* = gap between
  repeats, *mix* = how loud the repeats are.

## 22. ADC kickback on a high-resistance input
At the start of every conversion, the ADC connects its sampling capacitor to the pin and
dumps a little charge onto it. A low-resistance source (a ladder key, ≤ 3.25 kΩ) absorbs
that instantly. With no key touched, only the 100 kΩ pull-down drains it, which takes
microseconds, so the pin reads a false voltage. Measured on `P0_29` with nothing touched:

| Sample window | Reading |
|---|---|
| 3 clocks | ~175 |
| 35 clocks | ~135 |
| 67 clocks | ~105 |
| 131 clocks | ~60 |

Back-to-back averaging made it worse, because the charge piles up between conversions.
- **Firmware fix:** the longest sample window, one conversion per read.
- **Hardware fix:** a 100 nF capacitor from the pin to GND. It acts as a charge reservoir
  (5 pF dumped into 100 nF changes nothing) and also filters mains hum. The time
  constant 100 kΩ × 100 nF = 10 ms is still fast enough for lifting the stylus.

Rule: an ADC input wants a low-resistance source, or a capacitor right at the pin.

## 24. Shared wires break "ratiometric"
Ratiometric (entry 6) only works if the ladder's top really sits at the ADC reference
voltage. Adding the OLED onto the same breadboard + rail jumper made its current (~20 mA)
flow through that jumper and the rail's spring contacts, about 2 Ω together.
V = I × R = 40 mV lost, so the top keys read ~2 % low (B4 went from 3747 to 3681).
Rule: give a measurement circuit its own supply and ground wires, back to the source.
Current from other parts then can't shift its voltage. This is the same idea as the
star ground in entry 15.
How it was found: change one variable at a time (reference, sample time, averaging) and
watch the reading. Holding the stylus's bare metal swung it 0 ↔ 700 (mains hum through
your body), while touching GND read 0.

## 23. The original Stylophone (our reference)
1967, Dubreq. A metal (nickel-plated PCB) keyboard of about 20 keys spanning A to E,
about 1.5 octaves, played by touching a brass-tipped stylus to it. One square-wave tone,
a vibrato switch (~7 Hz wobble), a tuning knob underneath, a power switch, and a tiny
speaker. The modern GEN X-1 adds a sub-oscillator, low-pass filter, envelope, LFO and
delay; that's the feature set this build borrows.
