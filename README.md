# pico_led_strip

Raspberry Pi Pico firmware driving 4x WS2815 LED strips. See
[CLAUDE.md](CLAUDE.md) for the full architecture/animation docs and
[diagram.json](diagram.json) for the wiring.

## Prerequisites

- **Wokwi Simulator** VS Code extension -- runs [diagram.json](diagram.json)
  in-editor so the firmware can be simulated without real hardware.
- **CMake Tools** VS Code extension -- drives configure/build using the
  settings in [.vscode/settings.json](.vscode/settings.json).
- **GNU Arm Embedded Toolchain** (`arm-none-eabi-gcc`) -- this project uses
  the `mingw-w64-i686-arm-none-eabi` Windows build; install it and point
  `cmake.configureSettings.PICO_TOOLCHAIN_PATH` in
  [.vscode/settings.json](.vscode/settings.json) at its install directory.
- **MSYS2 UCRT64** -- provides the native (host, not ARM) GCC that CMake
  uses to build host-side tools (e.g. `pioasm`) during configure. Install
  [MSYS2](https://www.msys2.org/), then from an MSYS2 shell run
  `pacman -S mingw-w64-ucrt-x86_64-toolchain`, and make sure
  `cmake.environment.PATH` in [.vscode/settings.json](.vscode/settings.json)
  includes its `ucrt64/bin` directory.
- **Ninja** -- the build generator `cmake.generator` selects; install it and
  point `cmake.configureSettings.CMAKE_MAKE_PROGRAM` at `ninja.exe`.

After installing these, adjust the paths above in
[.vscode/settings.json](.vscode/settings.json) to match where they landed on
your machine, then clone the [pico-sdk](pico-sdk) submodule with
`git submodule update --init --recursive` (see [CLAUDE.md](CLAUDE.md)).

## Flashing from VS Code

1. Plug the Pico into USB (no need to hold BOOTSEL -- the firmware exposes
   a USB reset interface that `picotool` uses to reboot it into BOOTSEL
   mode automatically).
2. Open the Command Palette (`Ctrl+Shift+P`) and run **Tasks: Run Task**,
   then pick **Upload** -- or from the menu bar: **Terminal > Run Task... >
   Upload**.
3. This first runs **Build Release** (reconfigures `build/` with
   `CMAKE_BUILD_TYPE=Release` and builds `pico_led_strip.elf`), then flashes
   it with `picotool load -u -v -x -f`, which reboots the board into
   BOOTSEL, loads the firmware (only if it changed), verifies it, and
   reboots into the new firmware.

Other tasks (also reachable via **Tasks: Run Task**):

- **Build** -- just builds `build/` as currently configured (`Ctrl+Shift+B`,
  the default build task).
- **Configure Release** / **Build Release** -- (re)configure and build in
  Release without flashing.
