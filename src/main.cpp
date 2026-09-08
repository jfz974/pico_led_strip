#include <cstdint>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "ws2812_strip.h"

// 4x WS2815 LED strips, 400 LEDs each, driven over PIO with WS2812-compatible
// timing (WS2815 shares the same one-wire protocol as WS2812/NeoPixel; it
// only differs electrically by running at 12V and having a redundant backup
// data line, neither of which is modeled here).
//
// The strips are arranged in the diagram as 4 branches of a diagonal cross.
// Pixel index (kLedsPerStrip - 1), the far end from DIN, sits at the shared
// crossing point in the middle; index 0 (the DIN end) sits at the outer tip
// of each branch. This animation throws a white comet-style particle from
// that shared center down every branch at once, each with a fading tail
// behind it, counting DOWN from the last index to 0 as it travels outward.
//
// Each particle follows an S-curve speed profile: it eases in from a stop at
// the center, reaches peak speed around the midpoint of the branch, then
// eases back down to a stop right at the tip, instead of moving at a
// constant speed.

namespace {

constexpr uint kNumStrips = 4;
constexpr uint kLedsPerStrip = 400;
constexpr uint kStripPins[kNumStrips] = {2, 3, 4, 5};

constexpr uint64_t kSpawnIntervalUs = 500000; // 500 ms
constexpr float kTravelDurationS = 1.3f;      // seconds to cross the whole branch
constexpr int kTailLength = 20;               // pixels of fading tail behind the head
constexpr uint kMaxParticles = 8;             // headroom above kTravelDurationS / spawn interval

struct Particle {
	float age = 0.0f; // seconds since spawn
	bool active = false;
};

Particle particles[kMaxParticles];

// Smoothstep: 0 at s=0, 1 at s=1, zero slope (zero speed) at both ends and
// maximum slope (peak speed) at the midpoint -- the classic S-curve profile.
float ease_in_out(float s) {
	return s * s * (3.0f - 2.0f * s);
}

void spawn_particle() {
	for (auto &p : particles) {
		if (!p.active) {
			p.active = true;
			p.age = 0.0f;
			return;
		}
	}
}

void update_particles(float dt) {
	for (auto &p : particles) {
		if (!p.active) continue;
		p.age += dt;
		if (p.age >= kTravelDurationS) {
			p.active = false;
		}
	}
}

void render(Ws2812Strip<kLedsPerStrip> *strips[kNumStrips]) {
	for (uint i = 0; i < kNumStrips; i++) {
		strips[i]->clear();
	}

	for (auto &p : particles) {
		if (!p.active) continue;

		float s = p.age / kTravelDurationS; // 0 at spawn, 1 at the tip
		float eased = ease_in_out(s);
		float position = static_cast<float>(kLedsPerStrip - 1) * (1.0f - eased);
		int head = static_cast<int>(position);

		for (int t = 0; t <= kTailLength; t++) {
			int idx = head + t; // tail trails toward the center (higher index)
			if (idx < 0 || idx >= static_cast<int>(kLedsPerStrip)) continue;

			float fade = 1.0f - static_cast<float>(t) / kTailLength;
			fade *= fade; // ease-out tail
			uint8_t level = static_cast<uint8_t>(255.0f * fade);

			for (uint i = 0; i < kNumStrips; i++) {
				strips[i]->set_pixel(idx, level, level, level);
			}
		}
	}

	for (uint i = 0; i < kNumStrips; i++) {
		strips[i]->show();
	}
}

} // namespace

int main() {
	stdio_init_all();

	Ws2812Strip<kLedsPerStrip> strip0(kStripPins[0]);
	Ws2812Strip<kLedsPerStrip> strip1(kStripPins[1]);
	Ws2812Strip<kLedsPerStrip> strip2(kStripPins[2]);
	Ws2812Strip<kLedsPerStrip> strip3(kStripPins[3]);
	Ws2812Strip<kLedsPerStrip> *strips[kNumStrips] = {&strip0, &strip1, &strip2, &strip3};

	for (uint i = 0; i < kNumStrips; i++) {
		strips[i]->clear();
		strips[i]->show();
	}

	uint64_t last_spawn_us = time_us_64();
	uint64_t last_frame_us = last_spawn_us;

	while (true) {
		uint64_t now_us = time_us_64();

		if (now_us - last_spawn_us >= kSpawnIntervalUs) {
			last_spawn_us += kSpawnIntervalUs;
			spawn_particle();
		}

		float dt = static_cast<float>(now_us - last_frame_us) / 1e6f;
		last_frame_us = now_us;

		update_particles(dt);
		render(strips);

		sleep_ms(16); // ~60 fps refresh
	}
}
