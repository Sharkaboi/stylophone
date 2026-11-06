# Workshop notes

Hands-on technique: how to do and check each physical step. `LEARNING.md` explains why
things work; this file is the how.

## Safety and handling
- Touch grounded metal before handling boards. Hold boards by the edges and keep modules
  bagged until used.
- Never power new wiring untested. Check Ω between 5 V and GND first: low and climbing
  = capacitors charging (fine); stuck near 0 Ω = short.
- Power the board only through USB-C. It has no USB-PD chip, so it always receives 5 V,
  even from a 100 W charger. If a C-to-C cable leaves it dark, use an A-to-C cable.

## Multimeter (MAS830L)
- **DC V**: black probe on GND, red on the point. Ladder taps should step by 0.25 V. All
  0 or all 3.3 V = broken chain; uneven steps = wrong value or wrong column.
- **Continuity** (beep): finds shorts and proves connections. Use it on every new joint
  and between neighbouring pins.
- **Ω**: measure resistors instead of reading colour bands, out of circuit.
  1 kΩ 1 % reads 990–1010 Ω.

## Breadboard
- The long edge rails (red +, blue −) run the full length.
- Each short column of 5 holes in the middle is connected; the centre trench separates
  the two halves.
- Ladder layout: R1 in columns 1–2, R2 in 2–3 … R13 in 13–14. Column 1 → 3.3 V
  (= K13), column 14 → GND. Columns 1–13 are the 13 key taps.
- The FRDM's Arduino headers are female, so use M-M jumpers. Alligator clips hold pot lugs.

## Soldering
- Use a temperature-controlled iron (a sensor holds the set temperature). "Variable
  wattage" isn't the same: it sets power, so the tip still overheats or sags.
- **330 °C** for normal joints; **360–380 °C** for nickel keys and large copper.
- Heat the **pad and pin together** for 1–2 s, then feed solder into the joint, not onto
  the tip. The whole joint takes 2–3 s; longer cooks parts.
- Use flux. A good joint is a small shiny cone hugging the pin. Blobby = reflow with flux.
- Keep the tip tinned when parking it. A tip that won't wet: clean it with flux, scrub,
  re-tin within 2 s. Still dull = the tip is dead.
- Clean flux off with IPA (99 %) and a cotton bud.

## Header pins on modules
1. Push the long ends into the breadboard: it holds the pins straight.
2. Drop the module on, flux, tack one end pin, check the module sits level, tack the
   opposite end, then do the rest.
3. Beep-test neighbouring pins. Expected beeps: DAC GND–AGND.

## Wires, leads, heat-shrink
- **Solid** wire for anything that never moves; **stranded** for anything that flexes
  (solid work-hardens and snaps).
- **Half-jumper trick**: cut an F-F jumper in half, strip the cut end and solder it to the
  panel part (pot, button, speaker). The female end plugs onto the carrier's male pins,
  so nothing is permanent.
- Heat-shrink: slide it on *before* soldering; it shrinks to about half its diameter.
  Heat it with the iron barrel or hot air, never a flame up close.
- Speaker tabs: tin the tab and the wire separately, join them, then hot-glue for strain
  relief.

## Parts quirks
- **4-pin tactile button**: use two diagonally opposite pins.
- **Pot**: middle lug = wiper; swap the outer lugs to reverse direction.
- **3-pin PJ311 jack mapping**: insert a plug, then beep each pin to the plug's tip
  (left/signal), ring (right) and sleeve (GND).

## Nickel keys
1. Salt-water test a fresh-cut edge overnight: rust = a steel core.
2. Flatten the strip under books overnight, then clean it with IPA.
3. Cut 13 × 50 mm with a set square and snips, in one smooth cut each. Flatten again;
   sand the edges with 600–1000 grit.
4. On the underside of one end: scuff, flux, tin at ~380 °C. Join the pre-tinned wire in
   1–2 s, keeping the joint flat.
5. Tug-test, continuity-test, and label the wires 1–13.
6. Press each key into a printed pocket (8 mm wide, 2 mm walls, depth = strip + thin
   double-sided tape). Never lacquer the top surface.

## Ladder module
- Stand the resistors upright (hairpin) on perfboard. Pick the 13 that measure closest.
- A 14-pin header underneath carries GND, K1 … K12 and K13/3V3. The 100 kΩ pull-down
  stays off the module, at the stylus input.
- Before use: check for solder bridges with continuity, then confirm 0.25 V steps.

## Stylus
- Brass tip filed round and smooth, in a pen body or a printed handle.
- Stranded silicone wire with strain relief at both ends (knot, hot glue or a printed
  clip). The moulded plug from an old aux cable outlasts a soldered one.

## 3D-printed mounting
- Brass heat-set inserts (M3 × 8 × 4.2): print the hole at 4.0 mm, at least 1 mm deeper
  than the insert. Press in with the iron at 220 °C (PLA) or 240 °C (PETG). Posts ≥ 9 mm.
- Small modules: M2 screws into printed nut traps. Avoid nylon screws: they strip.
- Mock the panel layout in cardboard before modelling it.
