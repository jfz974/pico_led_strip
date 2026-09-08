#ifndef FIRE_ANIMATION_H
#define FIRE_ANIMATION_H

#include <cstdint>
#include <cstdlib>

#include "animation.h"
#include "pico/time.h"

// Fire2012-style heat simulation (cooling + upward drift + random sparking),
// adapted to the radial layout: the "base" of the flame is the shared
// center (kLedsPerStrip - 1), and heat drifts outward toward the tip (0),
// so each branch looks like a flame radiating out of the crossing point.
// All 4 branches share one heat simulation for a uniform look.
class FireAnimation : public Animation {
public:
	const char *get_name() const override { return "Fire"; }

	void start() override {
		srand(static_cast<unsigned>(time_us_32()));
		for (auto &h : heat_) {
			h = 0;
		}
		step_accumulator_ = 0.0f;
	}

	void update(float dt) override {
		step_accumulator_ += dt;
		while (step_accumulator_ >= kStepIntervalS) {
			step_accumulator_ -= kStepIntervalS;
			step_fire();
		}
	}

	void render(LedStrip *strips[kNumStrips]) override {
		for (uint i = 0; i < kLedsPerStrip; i++) {
			uint8_t r, g, b;
			heat_to_color(heat_[i], r, g, b);
			for (uint s = 0; s < kNumStrips; s++) {
				strips[s]->set_pixel(i, r, g, b);
			}
		}
	}

private:
	static constexpr float kStepIntervalS = 1.0f / 50.0f; // simulation rate
	static constexpr uint8_t kCooling = 55;
	static constexpr uint8_t kSparking = 120;

	static uint8_t qsub8(uint8_t a, uint8_t b) {
		return (a > b) ? static_cast<uint8_t>(a - b) : 0;
	}

	static uint8_t qadd8(uint8_t a, uint8_t b) {
		uint16_t sum = static_cast<uint16_t>(a) + b;
		return sum > 255 ? 255 : static_cast<uint8_t>(sum);
	}

	static void heat_to_color(uint8_t heat, uint8_t &r, uint8_t &g, uint8_t &b) {
		uint8_t t192 = static_cast<uint8_t>((static_cast<uint16_t>(heat) * 191) / 255);
		uint8_t heatramp = static_cast<uint8_t>((t192 & 0x3F) << 2);

		if (t192 > 0x80) { // hottest: yellow -> white
			r = 255;
			g = 255;
			b = heatramp;
		} else if (t192 > 0x40) { // middle: red -> yellow
			r = 255;
			g = heatramp;
			b = 0;
		} else { // coolest: black -> red
			r = heatramp;
			g = 0;
			b = 0;
		}
	}

	void step_fire() {
		constexpr int n = static_cast<int>(kLedsPerStrip);

		// Cool down every cell a little.
		for (int i = 0; i < n; i++) {
			heat_[i] = qsub8(heat_[i], static_cast<uint8_t>(rand() % (((kCooling * 10) / n) + 2)));
		}

		// Heat drifts from the center (high index) out toward the tip (low
		// index) and diffuses a little.
		for (int k = 0; k <= n - 3; k++) {
			heat_[k] = static_cast<uint8_t>((heat_[k + 1] + heat_[k + 2] + heat_[k + 2]) / 3);
		}

		// Randomly ignite new sparks near the center.
		if ((rand() % 255) < kSparking) {
			int y = n - 1 - (rand() % 7);
			heat_[y] = qadd8(heat_[y], static_cast<uint8_t>(160 + rand() % 96));
		}
	}

	uint8_t heat_[kLedsPerStrip] = {0};
	float step_accumulator_ = 0.0f;
};

#endif
