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

## Frame rate

The main loop runs at **~15 fps, not ~60** as the `sleep_ms(16)` in
[src/main.cpp](src/main.cpp) might suggest: `show()` on all 4 strips blocks
for ~48ms/frame on its own (400 LEDs * 24 bits * 1.25us/bit per strip,
times 4, sequentially -- WS2812 is a fixed-rate serial protocol, there's no
way to push pixels out faster). Adding the 16ms sleep on top yields
~64ms/frame. Keep this in mind when tuning animation timing: durations
expressed in seconds (most animations) already account for this via `dt`,
but anything counted in frames (see `FastTwinkleAnimation` below) advances
once per `update()` call regardless of wall-clock time, so its speed is
inherently tied to this ~15 fps rate.

## Animation pattern

Animations follow a small Strategy pattern so new ones can be dropped in
without touching the main loop:

- [src/animation/animation.h](src/animation/animation.h) defines the
  `Animation` interface:
  - `get_name()` -- short human-readable name, used for logging which
    animation is active.
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
  - [rotating_sweep_animation.h](src/animation/rotating_sweep_animation.h) -- `RotatingSweepAnimation`,
    a radar-like beam continuously rotating branch to branch.
  - [meteor_rain_animation.h](src/animation/meteor_rain_animation.h) -- `MeteorRainAnimation`,
    per-branch meteors at random intervals/speed/brightness, constant speed
    (no easing) -- an ongoing "rain" vs. `ParticleAnimation`'s single synced
    pulse.
  - [color_meteor_rain_animation.h](src/animation/color_meteor_rain_animation.h) -- `ColorMeteorRainAnimation`,
    a `MeteorRainAnimation` variant: each meteor gets its own random hue
    (via [color.h](src/color.h)) instead of white, a wider speed spread, and
    a denser spawn rate/more concurrent meteors.
  - [rainbow_cycle_animation.h](src/animation/rainbow_cycle_animation.h) -- `RainbowCycleAnimation`,
    a hue gradient along each branch that drifts over time.
  - [theater_chase_animation.h](src/animation/theater_chase_animation.h) -- `TheaterChaseAnimation`,
    classic every-3rd-LED marquee chase, cycling colors each pass.
  - [larson_scanner_animation.h](src/animation/larson_scanner_animation.h) -- `LarsonScannerAnimation`,
    the Knight Rider red eye, constant-speed (no easing) bounce -- deliberately
    snappier than `PingPongAnimation`'s S-curve bounce.
  - [twinkle_animation.h](src/animation/twinkle_animation.h) -- `TwinkleAnimation`,
    random confetti-style sparkles that flash and fade on a dark background.
  - [fast_twinkle_animation.h](src/animation/fast_twinkle_animation.h) -- `FastTwinkleAnimation`,
    up to 150 unsynchronized white points (8% chance per dark slot per frame
    to ignite), each with a quick 3-4 frame attack and slower 6-8 frame
    release, dimly lighting each point's immediate neighbor pixels at 20% of
    its intensity. New points must land at least 3 pixels from every other
    active point on the same branch. The envelope is counted in actual
    `update()` calls rather than wall-clock time (see "Frame rate" above),
    so its pacing is tied to the main loop's rate rather than real seconds.
  - [fire_animation.h](src/animation/fire_animation.h) -- `FireAnimation`,
    a Fire2012-style heat simulation (cooling + drift + sparking), flame
    radiating outward from the shared center.
  - [breathing_animation.h](src/animation/breathing_animation.h) -- `BreathingAnimation`,
    one calm color fading in/out on a slow sine curve -- a single even
    breath, vs. `HeartbeatAnimation`'s sharp double-pulse.
  - [wave_animation.h](src/animation/wave_animation.h) -- `WaveAnimation`, a
    single wave continuously sweeping center->tip and looping, playing on
    one branch at a time and alternating to the next branch each pass
    (never more than one branch lit at once); the wave is 60% of the strip's
    length wide, with intensity across that width following one half period
    of a sine wave (0 at both edges, peak in the middle).

  Use these as the reference for how a new animation should be structured:
  all state (particle pools, timers, phase, etc.) lives as private members,
  not file-scope globals, so multiple animations can coexist without
  stepping on each other. Shared per-frame math lives outside
  `src/animation/` (it's generic, not itself an animation) rather than being
  duplicated in each animation that needs it: the S-curve easing function in
  [src/easing.h](src/easing.h), and HSV-to-RGB conversion in
  [src/color.h](src/color.h).

- [src/main.cpp](src/main.cpp) owns the 4 `LedStrip` instances, an array of
  `Animation*` (one per concrete animation above), and a `current_animation`
  index. Each frame it calls `update(dt)` then `render(strips)` on the
  active animation, then calls `show()` on every strip itself -- animations
  never push pixels to the hardware directly.

### Selecting animations (button + auto-switch timer)

`current_animation` can change two ways:

- **Button**: a push button on `kButtonPin` (GP6, wired to GND with the
  pin's internal pull-up -- see `btn1` in [diagram.json](diagram.json)) is
  debounced in software by [src/debounced_button.h](src/debounced_button.h).
  Each `DebouncedButton::consume_press()` returning `true` (once per
  physical press) advances to the next entry in the array, wrapping around.
- **Timer**: if `kAutoSwitchIntervalUs` (2 minutes) passes with no button
  press, `main()` auto-switches to a *random* animation, guaranteed
  different from the current one (`pick_different_animation()` picks a
  random nonzero offset into the array rather than rejection-sampling).
  A button press resets this timer too, so it's really "2 minutes since the
  last switch of either kind," not a fixed wall-clock schedule.

Loading an animation (the initial selection and every subsequent switch,
manual or automatic) goes through `main.cpp`'s `load_animation()` helper,
which calls `start()` and then `printf`s `get_name()` over stdio (USB/UART)
so the active animation is visible in the serial monitor.

### Adding a new animation

1. Create `src/animation/<name>_animation.h` with a class `<Name>Animation : public Animation`
   implementing `get_name()`, `update()`, and `render()` (and `start()` if it
   needs to reset state). Reuse [easing.h](src/easing.h) if it needs an
   S-curve, or [color.h](src/color.h) if it needs HSV-to-RGB.
2. `#include` it in `main.cpp`, add an instance, and append a pointer to it
   in the `animations[]` array -- the button (and the load-time log) will
   pick it up automatically.
