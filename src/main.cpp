#include "pico/stdlib.h"
#include "pico/time.h"

#include "animation.h"
#include "particle_animation.h"

// 4x WS2815 LED strips, 400 LEDs each, driven over PIO with WS2812-compatible
// timing (WS2815 shares the same one-wire protocol as WS2812/NeoPixel; it
// only differs electrically by running at 12V and having a redundant backup
// data line, neither of which is modeled here).
//
// LED animations implement the Animation interface (animation.h); main()
// just drives whichever one is currently active, so new animations can be
// dropped in without touching this file's loop.

namespace {

constexpr uint kStripPins[kNumStrips] = {2, 3, 4, 5};

} // namespace

int main() {
	stdio_init_all();

	LedStrip strip0(kStripPins[0]);
	LedStrip strip1(kStripPins[1]);
	LedStrip strip2(kStripPins[2]);
	LedStrip strip3(kStripPins[3]);
	LedStrip *strips[kNumStrips] = {&strip0, &strip1, &strip2, &strip3};

	for (uint i = 0; i < kNumStrips; i++) {
		strips[i]->clear();
		strips[i]->show();
	}

	ParticleAnimation particle_animation;
	Animation *animation = &particle_animation;
	animation->start();

	uint64_t last_frame_us = time_us_64();

	while (true) {
		uint64_t now_us = time_us_64();
		float dt = static_cast<float>(now_us - last_frame_us) / 1e6f;
		last_frame_us = now_us;

		animation->update(dt);
		animation->render(strips);

		for (uint i = 0; i < kNumStrips; i++) {
			strips[i]->show();
		}

		sleep_ms(16); // ~60 fps refresh
	}
}
