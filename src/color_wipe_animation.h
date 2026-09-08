#ifndef COLOR_WIPE_ANIMATION_H
#define COLOR_WIPE_ANIMATION_H

#include <cstdint>

#include "animation.h"

// Classic color wipe, adapted to the radial layout: a solid color fills
// outward from the center (pixel index kLedsPerStrip - 1) to the tip (pixel
// index 0) on every branch at once, holds fully lit, wipes back out to dark
// in the same direction, holds dark, then repeats with the next color in a
// small palette.
class ColorWipeAnimation : public Animation {
public:
	void start() override {
		phase_ = Phase::kFill;
		time_in_phase_ = 0.0f;
		color_index_ = 0;
	}

	void update(float dt) override {
		time_in_phase_ += dt;

		switch (phase_) {
			case Phase::kFill:
				if (time_in_phase_ >= kFillDurationS) {
					phase_ = Phase::kHoldOn;
					time_in_phase_ = 0.0f;
				}
				break;
			case Phase::kHoldOn:
				if (time_in_phase_ >= kHoldDurationS) {
					phase_ = Phase::kWipe;
					time_in_phase_ = 0.0f;
				}
				break;
			case Phase::kWipe:
				if (time_in_phase_ >= kWipeDurationS) {
					phase_ = Phase::kHoldOff;
					time_in_phase_ = 0.0f;
				}
				break;
			case Phase::kHoldOff:
				if (time_in_phase_ >= kHoldDurationS) {
					phase_ = Phase::kFill;
					time_in_phase_ = 0.0f;
					color_index_ = (color_index_ + 1) % kPaletteSize;
				}
				break;
		}
	}

	void render(LedStrip *strips[kNumStrips]) override {
		uint8_t r, g, b;
		palette_color(color_index_, r, g, b);

		constexpr int kLastIdx = static_cast<int>(kLedsPerStrip) - 1;

		for (uint s = 0; s < kNumStrips; s++) {
			for (int idx = 0; idx < static_cast<int>(kLedsPerStrip); idx++) {
				bool lit;
				switch (phase_) {
					case Phase::kFill: {
						float f = time_in_phase_ / kFillDurationS;
						int front_idx = static_cast<int>(kLastIdx * (1.0f - f));
						lit = idx >= front_idx; // grows from center toward tip
						break;
					}
					case Phase::kHoldOn:
						lit = true;
						break;
					case Phase::kWipe: {
						float w = time_in_phase_ / kWipeDurationS;
						int front_idx = static_cast<int>(kLastIdx * (1.0f - w));
						lit = idx < front_idx; // clears from center toward tip
						break;
					}
					case Phase::kHoldOff:
					default:
						lit = false;
						break;
				}

				if (lit) {
					strips[s]->set_pixel(idx, r, g, b);
				} else {
					strips[s]->set_pixel(idx, 0, 0, 0);
				}
			}
		}
	}

private:
	enum class Phase { kFill, kHoldOn, kWipe, kHoldOff };

	static constexpr float kFillDurationS = 0.8f;
	static constexpr float kWipeDurationS = 0.8f;
	static constexpr float kHoldDurationS = 0.3f;
	static constexpr uint kPaletteSize = 4;

	static void palette_color(uint index, uint8_t &r, uint8_t &g, uint8_t &b) {
		switch (index % kPaletteSize) {
			case 0: r = 255; g = 40; b = 20; break;  // warm red
			case 1: r = 20; g = 180; b = 255; break; // cyan-blue
			case 2: r = 40; g = 220; b = 60; break;  // green
			default: r = 255; g = 160; b = 10; break; // amber
		}
	}

	Phase phase_ = Phase::kFill;
	float time_in_phase_ = 0.0f;
	uint color_index_ = 0;
};

#endif
