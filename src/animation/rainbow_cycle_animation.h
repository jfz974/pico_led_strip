#ifndef RAINBOW_CYCLE_ANIMATION_H
#define RAINBOW_CYCLE_ANIMATION_H

#include <cmath>
#include <cstdint>

#include "animation.h"
#include "color.h"

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

	float hue_offset_ = 0.0f;
};

#endif
