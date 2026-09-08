#ifndef BREATHING_ANIMATION_H
#define BREATHING_ANIMATION_H

#include <cmath>
#include <cstdint>

#include "animation.h"

// A single calm color slowly fades in and out on a smooth sine curve,
// across the whole cross at once. Distinct from HeartbeatAnimation's sharp
// double-pulse "lub-dub" envelope -- this is a single, slow, even breath.
class BreathingAnimation : public Animation {
public:
	const char *get_name() const override { return "Breathing"; }

	void start() override {
		time_ = 0.0f;
	}

	void update(float dt) override {
		time_ += dt;
	}

	void render(LedStrip *strips[kNumStrips]) override {
		float phase = fmodf(time_, kPeriodS) / kPeriodS; // 0..1
		float raw = 0.5f - 0.5f * cosf(phase * kTwoPi);  // 0..1, smooth in/out
		float level = kMinLevel + (1.0f - kMinLevel) * raw;

		uint8_t r = static_cast<uint8_t>(kColorR * level);
		uint8_t g = static_cast<uint8_t>(kColorG * level);
		uint8_t b = static_cast<uint8_t>(kColorB * level);

		for (uint s = 0; s < kNumStrips; s++) {
			for (uint i = 0; i < kLedsPerStrip; i++) {
				strips[s]->set_pixel(i, r, g, b);
			}
		}
	}

private:
	static constexpr float kTwoPi = 6.28318530718f;
	static constexpr float kPeriodS = 4.0f;
	static constexpr float kMinLevel = 0.05f;
	static constexpr uint8_t kColorR = 40;
	static constexpr uint8_t kColorG = 120;
	static constexpr uint8_t kColorB = 255; // calm blue

	float time_ = 0.0f;
};

#endif
