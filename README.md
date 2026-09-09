# pico_led_strip

Raspberry Pi Pico firmware driving 4x WS2815 LED strips. See
[CLAUDE.md](CLAUDE.md) for the full architecture/animation docs and
[diagram.json](diagram.json) for the wiring.

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
