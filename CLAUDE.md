# pico_led_strip

Raspberry Pi Pico firmware driving 4x WS2815 LED strips (400 LEDs each),
wired as 4 branches of a diagonal cross meeting at pixel index
`kLedsPerStrip - 1` (the far end from DIN); pixel index `0` (the DIN end)
sits at the outer tip of each branch. See [diagram.json](diagram.json) for
the Wokwi wiring/layout.

## Build

Plain pico-sdk + CMake + Ninja (no PlatformIO/Arduino) -- see
[project/CMakeLists.txt](project/CMakeLists.txt). This machine's toolchain
paths (shared pico-sdk checkout, arm-none-eabi-gcc, ninja, prebuilt
picotool) are pinned in [.vscode/settings.json](.vscode/settings.json)'s
`cmake.configureSettings`. Build via the VS Code CMake Tools extension, or
the "Build" task in [.vscode/tasks.json](.vscode/tasks.json), which runs
`cmake --build build`.

## Animation pattern

Animations follow a small Strategy pattern so new ones can be dropped in
without touching the main loop:

- [src/animation.h](src/animation.h) defines the `Animation` interface:
  - `start()` -- (re)initializes the animation's state; called once when it
    becomes active.
  - `update(dt)` -- advances internal state by `dt` seconds.
  - `render(strips)` -- writes this frame's pixel data into each strip's
    buffer via `set_pixel`/`clear`. Does **not** call `show()`.
  - It also declares the shared hardware layout every animation targets:
    `kNumStrips`, `kLedsPerStrip`, and the `LedStrip` alias
    (`Ws2812Strip<kLedsPerStrip>`, from [src/ws2812_strip.h](src/ws2812_strip.h)).

- [src/particle_animation.h](src/particle_animation.h) is the first (and so
  far only) concrete animation, `ParticleAnimation : public Animation`. Use
  it as the reference for how a new animation should be structured: all of
  its state (particle pool, spawn timer, etc.) lives as private members, not
  file-scope globals, so multiple animations can coexist without stepping on
  each other.

- [src/main.cpp](src/main.cpp) owns the 4 `LedStrip` instances and a single
  `Animation *animation` pointer. Each frame it calls
  `animation->update(dt)`, then `animation->render(strips)`, then calls
  `show()` on every strip itself -- animations never push pixels to the
  hardware directly.

### Adding a new animation

1. Create `src/<name>_animation.h` with a class `<Name>Animation : public Animation`
   implementing `update()` and `render()` (and `start()` if it needs to reset
   state).
2. `#include` it in `main.cpp`.
3. Point `animation` at an instance of it (currently hardcoded to
   `particle_animation`) -- swapping which animation runs today is a one-line
   change. Switching between several at runtime (e.g. on a timer or button
   press) would just mean cycling through an array of `Animation*` instead
   of a single pointer; that hasn't been needed yet since only one animation
   exists.
