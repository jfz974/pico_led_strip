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

- [src/animation/animation.h](src/animation/animation.h) defines the
  `Animation` interface:
  - `start()` -- (re)initializes the animation's state; called once when it
    becomes active.
  - `update(dt)` -- advances internal state by `dt` seconds.
  - `render(strips)` -- writes this frame's pixel data into each strip's
    buffer via `set_pixel`/`clear`. Does **not** call `show()`.
  - It also declares the shared hardware layout every animation targets:
    `kNumStrips`, `kLedsPerStrip`, and the `LedStrip` alias
    (`Ws2812Strip<kLedsPerStrip>`, from [src/ws2812_strip.h](src/ws2812_strip.h)).

- Concrete animations, each in its own header, all under `src/animation/`:
  - [particle_animation.h](src/animation/particle_animation.h) -- `ParticleAnimation`,
    a white comet spawned every 500 ms at the center, S-curve speed out to
    the tip.
  - [heartbeat_animation.h](src/animation/heartbeat_animation.h) -- `HeartbeatAnimation`,
    the whole cross pulses together in a "lub-dub" brightness envelope.
  - [color_wipe_animation.h](src/animation/color_wipe_animation.h) -- `ColorWipeAnimation`,
    a solid color fills center->tip, holds, wipes back to dark, then cycles
    to the next color in a small palette.
  - [ping_pong_animation.h](src/animation/ping_pong_animation.h) -- `PingPongAnimation`,
    a single dot with a two-sided fading trail bouncing between center and
    tip on the same S-curve profile.

  Use these as the reference for how a new animation should be structured:
  all state (particle pools, timers, phase, etc.) lives as private members,
  not file-scope globals, so multiple animations can coexist without
  stepping on each other. Shared per-frame math (the S-curve easing
  function) lives in [src/easing.h](src/easing.h) (deliberately kept outside
  `src/animation/` since it's a generic math helper, not itself an
  animation) rather than being duplicated in each animation that needs it.

- [src/main.cpp](src/main.cpp) owns the 4 `LedStrip` instances, an array of
  `Animation*` (one per concrete animation above), and a `current_animation`
  index. Each frame it calls `update(dt)` then `render(strips)` on the
  active animation, then calls `show()` on every strip itself -- animations
  never push pixels to the hardware directly.

### Selecting animations (button)

A push button on `kButtonPin` (GP6, wired to GND with the pin's internal
pull-up -- see `btn1` in [diagram.json](diagram.json)) is debounced in
software by [src/debounced_button.h](src/debounced_button.h). Each
`DebouncedButton::consume_press()` returning `true` (once per physical
press) advances `current_animation` to the next entry in the array, wrapping
around, and calls `start()` on the newly selected animation.

### Adding a new animation

1. Create `src/animation/<name>_animation.h` with a class `<Name>Animation : public Animation`
   implementing `update()` and `render()` (and `start()` if it needs to reset
   state). Reuse [easing.h](src/easing.h) if it needs an S-curve.
2. `#include` it in `main.cpp`, add an instance, and append a pointer to it
   in the `animations[]` array -- the button will pick it up automatically.
