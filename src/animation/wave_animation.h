#ifndef WAVE_ANIMATION_H
#define WAVE_ANIMATION_H

#include <cmath>
#include <cstdint>

#include "animation.h"

// A single wave continuously sweeps from the center out to the tip and
// loops, playing on one branch at a time -- alternating to the next branch
// each time it completes a pass, rather than all 4 branches at once. The
// wave is 60% of the strip's length wide, and its intensity across that
// width follows one half period of a sine wave (0 at both edges, peak in
// the middle) rather than a hard-edged block.
class WaveAnimation : public Animation {
public:
	const char *get_name() const override { return "Wave"; }

	void start() override {
		position_ = kTravelStart;
		active_strip_ = 0;
	}

	void update(float dt) override {
		position_ -= kSpeed * dt;
		if (position_ <= kTravelEnd) {
			position_ = kTravelStart;
			active_strip_ = (active_strip_ + 1) % kNumStrips;
		}
	}

	void render(LedStrip *strips[kNumStrips]) override {
		for (uint s = 0; s < kNumStrips; s++) {
			if (s != active_strip_) {
				strips[s]->clear();
			}
		}

		for (uint i = 0; i < kLedsPerStrip; i++) {
			float x = (static_cast<float>(i) - position_) / kWaveWidth; // 0..1 across the wave
			float intensity = 0.0f;
			if (x >= 0.0f && x < 1.0f) {
				intensity = sinf(kPi * x); // half sine period: 0 at edges, 1 at the middle
			}

			uint8_t r = static_cast<uint8_t>(kColorR * intensity);
			uint8_t g = static_cast<uint8_t>(kColorG * intensity);
			uint8_t b = static_cast<uint8_t>(kColorB * intensity);
			strips[active_strip_]->set_pixel(i, r, g, b);
		}
	}

private:
	static constexpr float kPi = 3.14159265358979323846f;
	static constexpr float kWaveWidth = kLedsPerStrip / 2.0f * 1.2f; // half the strip, 20% wider
	static constexpr float kSpeed = 350.0f;                          // LEDs per second
	static constexpr float kTravelStart = static_cast<float>(kLedsPerStrip - 1) + kWaveWidth;
	static constexpr float kTravelEnd = -kWaveWidth;

	static constexpr uint8_t kColorR = 40;
	static constexpr uint8_t kColorG = 160;
	static constexpr uint8_t kColorB = 255; // ocean-wave blue

	float position_ = kTravelStart;
	uint active_strip_ = 0;
};

#endif
