#include <cstdio>

#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#include "animation/animation.h"
#include "animation/color_wipe_animation.h"
#include "animation/heartbeat_animation.h"
#include "animation/particle_animation.h"
#include "animation/ping_pong_animation.h"
#include "debounced_button.h"

// 4x WS2815 LED strips, 400 LEDs each, driven over PIO with WS2812-compatible
// timing (WS2815 shares the same one-wire protocol as WS2812/NeoPixel; it
// only differs electrically by running at 12V and having a redundant backup
// data line, neither of which is modeled here).
//
// LED animations implement the Animation interface (animation.h); main()
// drives whichever one is currently active and cycles to the next one on
// each button press, so new animations can be dropped into the list below
// without touching the rest of this file's loop.

namespace {

constexpr uint kStripPins[kNumStrips] = {2, 3, 4, 5};
constexpr uint kButtonPin = 6;

void load_animation(Animation *animation) {
	animation->start();
	printf("Loaded animation: %s\n", animation->get_name());
}

} // namespace

int main() {
	stdio_init_all();

	// Give the host up to ~2s to open the USB CDC port so early log lines
	// (like the startup message right below) aren't lost before a terminal
	// attaches. Proceeds regardless once the deadline passes.
	absolute_time_t usb_wait_deadline = make_timeout_time_ms(2000);
	while (!stdio_usb_connected() && !time_reached(usb_wait_deadline)) {
		sleep_ms(10);
	}

	printf("pico_led_strip: starting\n");

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
	HeartbeatAnimation heartbeat_animation;
	ColorWipeAnimation color_wipe_animation;
	PingPongAnimation ping_pong_animation;

	Animation *animations[] = {
		&particle_animation,
		&heartbeat_animation,
		&color_wipe_animation,
		&ping_pong_animation,
	};
	constexpr uint kNumAnimations = sizeof(animations) / sizeof(animations[0]);

	uint current_animation = 0;
	load_animation(animations[current_animation]);

	DebouncedButton button(kButtonPin);

	uint64_t last_frame_us = time_us_64();

	while (true) {
		uint64_t now_us = time_us_64();
		float dt = static_cast<float>(now_us - last_frame_us) / 1e6f;
		last_frame_us = now_us;

		if (button.consume_press()) {
			current_animation = (current_animation + 1) % kNumAnimations;
			load_animation(animations[current_animation]);
		}

		Animation *animation = animations[current_animation];
		animation->update(dt);
		animation->render(strips);

		for (uint i = 0; i < kNumStrips; i++) {
			strips[i]->show();
		}

		sleep_ms(16); // ~60 fps refresh
	}
}
