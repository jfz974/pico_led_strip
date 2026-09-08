#ifndef RAINBOW_CYCLE_ANIMATION_H
#define RAINBOW_CYCLE_ANIMATION_H

#include <cmath>
#include <cstdint>

#include "animation.h"

// Classic rainbow cycle: hue shifts smoothly along each strip (one full
// hue wheel per branch) and drifts over time, identical across all 4
// branches.
class RainbowCycleAnimation : public Animation {
public:
	const char *get_name() const override { return "Rainbow Cycle"; }

	void start() override {
		hue_offset_ = 0.0f;
	}

	void update(float dt) override {
		hue_offset_ += dt * kCycleSpeed;
		hue_offset_ -= floorf(hue_offset_);
	}

	void render(LedStrip *strips[kNumStrips]) override {
		for (uint i = 0; i < kLedsPerStrip; i++) {
			float hue = hue_offset_ + static_cast<float>(i) / static_cast<float>(kLedsPerStrip);
			hue -= floorf(hue);

			uint8_t r, g, b;
			hsv_to_rgb(hue, r, g, b);

			for (uint s = 0; s < kNumStrips; s++) {
				strips[s]->set_pixel(i, r, g, b);
			}
		}
	}

private:
	static constexpr float kCycleSpeed = 0.15f; // full hue cycles per second

	// Full-saturation, full-value HSV to RGB (h in [0,1)).
	static void hsv_to_rgb(float h, uint8_t &r, uint8_t &g, uint8_t &b) {
		float h6 = h * 6.0f;
		int i = static_cast<int>(h6);
		float f = h6 - static_cast<float>(i);
		float q = 1.0f - f;
		float t = f;

		float rf, gf, bf;
		switch (i % 6) {
			case 0: rf = 1.0f; gf = t; bf = 0.0f; break;
			case 1: rf = q; gf = 1.0f; bf = 0.0f; break;
			case 2: rf = 0.0f; gf = 1.0f; bf = t; break;
			case 3: rf = 0.0f; gf = q; bf = 1.0f; break;
			case 4: rf = t; gf = 0.0f; bf = 1.0f; break;
			default: rf = 1.0f; gf = 0.0f; bf = q; break;
		}

		r = static_cast<uint8_t>(rf * 255.0f);
		g = static_cast<uint8_t>(gf * 255.0f);
		b = static_cast<uint8_t>(bf * 255.0f);
	}

	float hue_offset_ = 0.0f;
};

#endif
