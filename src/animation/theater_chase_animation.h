#ifndef THEATER_CHASE_ANIMATION_H
#define THEATER_CHASE_ANIMATION_H

#include <cstdint>

#include "animation.h"

// Classic theater marquee chase: every 3rd LED lit, the lit set shifting by
// one pixel on a fixed step interval, cycling through a small color
// palette each time it completes a full spacing cycle.
class TheaterChaseAnimation : public Animation {
public:
	const char *get_name() const override { return "Theater Chase"; }

	void start() override {
		time_ = 0.0f;
		offset_ = 0;
		color_index_ = 0;
	}

	void update(float dt) override {
		time_ += dt;
		if (time_ >= kStepIntervalS) {
			time_ -= kStepIntervalS;
			offset_ = (offset_ + 1) % kSpacing;
			if (offset_ == 0) {
				color_index_ = (color_index_ + 1) % kPaletteSize;
			}
		}
	}

	void render(LedStrip *strips[kNumStrips]) override {
		uint8_t r, g, b;
		palette_color(color_index_, r, g, b);

		for (uint i = 0; i < kLedsPerStrip; i++) {
			bool lit = (i % kSpacing) == static_cast<uint>(offset_);
			for (uint s = 0; s < kNumStrips; s++) {
				if (lit) {
					strips[s]->set_pixel(i, r, g, b);
				} else {
					strips[s]->set_pixel(i, 0, 0, 0);
				}
			}
		}
	}

private:
	static constexpr float kStepIntervalS = 0.08f;
	static constexpr uint kSpacing = 3;
	static constexpr uint kPaletteSize = 3;

	static void palette_color(uint index, uint8_t &r, uint8_t &g, uint8_t &b) {
		switch (index % kPaletteSize) {
			case 0: r = 255; g = 255; b = 255; break; // white
			case 1: r = 255; g = 80; b = 0; break;    // amber
			default: r = 0; g = 120; b = 255; break;  // blue
		}
	}

	float time_ = 0.0f;
	int offset_ = 0;
	uint color_index_ = 0;
};

#endif
