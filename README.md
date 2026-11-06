# Stylophone on FRDM-MCXN236

A 13-key stylus synth built on NXP's FRDM-MCXN236 board. A brass stylus touches nickel
keys wired as a resistor ladder, and the board turns the voltage into notes through an
I2S DAC. It adds synth controls, optional microSD samples, and an OLED.

Work in progress, built stage by stage. Status and full context: [CLAUDE.md](CLAUDE.md).

## Layout
| Path | What |
|---|---|
| `firmware/` | MCUXpresso SDK (CMake) project, in C |
| `docs/DECISIONS.md` | Every design choice and why |
| `docs/MCXN236-Stylophone-Wiring-Plan.md` | What connects to what |
| `docs/FRDM-MCXN236-Header-Pinout.md` | Header pins, from the board netlist |
| `docs/MCXN236-Stylophone-BOM.csv` | Parts |
| `docs/LEARNING.md` · `docs/WORKSHOP.md` | Concepts · hands-on technique |

## Setup (Windows)
1. Install **VS Code** and the **MCUXpresso for VS Code** extension. In the MCUXpresso
   Installer, add:
   - MCUXpresso SDK Developer
   - Arm GNU Toolchain 14.2.1
   - LinkServer
   - cmake
   - ninja
2. Import the SDK for board `frdmmcxn236` (west flow).
3. Create the two machine-local files (git-ignored), with your own paths.

   `firmware/mcux_include.json`:
   ```json
   {
     "version": 7,
     "configurePresets": [
       { "name": "debug-env", "hidden": true, "environment": {
           "ARMGCC_DIR": "<home>/.mcuxpressotools/arm-gnu-toolchain-14.2.rel1-mingw-w64-x86_64-arm-none-eabi",
           "SdkRootDirPath": "<sdk path>",
           "MCUX_VENV_PATH": "<home>/.mcuxpressotools/.mcux-venv-3.12/Scripts",
           "PATH": "$env{MCUX_VENV_PATH}${pathListSep}$penv{PATH}",
           "POSTPROCESS_UTILITY": "<home>/.mcuxpressotools/mcux-fixelf-14.2.2/mcux-fixelf.exe",
           "CMAKE_EXPORT_COMPILE_COMMANDS": "ON" } },
       { "name": "release-env", "hidden": true, "inherits": "debug-env" }
     ],
     "buildPresets": []
   }
   ```
   `firmware/.vscode/mcuxpresso-tools.json`:
   ```json
   {
     "version": "25.3",
     "toolchainPath": "${userHome}/.mcuxpressotools/arm-gnu-toolchain-14.2.rel1-mingw-w64-x86_64-arm-none-eabi",
     "linkedProjects": [], "trustZoneType": "none", "multicoreType": "none",
     "projectType": "sdk-v2-freestanding", "projectName": "firmware",
     "sdk": { "path": "<sdk path>", "version": "26.6.0", "boardId": "frdmmcxn236",
              "deviceId": "MCXN236", "coreId": "cm33_core0" }
   }
   ```
4. Open the repo root in VS Code, build the `debug` preset, and flash over the
   MCU-Link USB-C port (J10). Serial: COM port @ 115200, then press SW1 (reset).

Board manuals and design files (UM12041, schematic, netlist): NXP's
[FRDM-MCXN236 page](https://www.nxp.com/design/design-center/development-boards-and-designs/FRDM-MCXN236).

## License
Apache-2.0. Files from NXP's SDK keep their own BSD-3-Clause headers.
