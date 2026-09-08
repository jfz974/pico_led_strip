#ifndef ROTATING_SWEEP_ANIMATION_H
#define ROTATING_SWEEP_ANIMATION_H

#include <cmath>
#include <cstdint>

#include "animation.h"

// A single bright "beam" rotates around the 4 branches like a radar sweep
// or clock hand, lighting whichever branch (or pair of adjacent branches,
// mid-transition) it currently points at, and dimming the rest.
class RotatingSweepAnimation : public Animation {
public:
	const char *get_name() const override { return "Rotating Sweep"; }

	void start() override {
		time_ = 0.0f;
	}

	void update(float dt) override {
		time_ += dt;
		float full_turn = kNumStrips * kBranchDurationS;
		if (time_ >= full_turn) {
			time_ -= full_turn;
		}
	}

	void render(LedStrip *strips[kNumStrips]) override {
		float position = time_ / kBranchDurationS; // 0..kNumStrips, current beam angle

		for (uint s = 0; s < kNumStrips; s++) {
			float dist = circular_distance(position, static_cast<float>(s));
			float level = 1.0f - dist / kBeamWidth;
			if (level < 0.0f) level = 0.0f;
			level *= level; // ease the falloff

			uint8_t v = static_cast<uint8_t>(255.0f * level);
			for (uint i = 0; i < kLedsPerStrip; i++) {
				strips[s]->set_pixel(i, v, v, v);
			}
		}
	}

private:
	static constexpr float kBranchDurationS = 0.35f; // time to sweep past one branch
	static constexpr float kBeamWidth = 1.0f;        // in branch-index units

	static float circular_distance(float a, float b) {
		float d = fmodf(fabsf(a - b), static_cast<float>(kNumStrips));
		float half = static_cast<float>(kNumStrips) / 2.0f;
		return d > half ? static_cast<float>(kNumStrips) - d : d;
	}

	float time_ = 0.0f;
};

#endif
