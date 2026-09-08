#ifndef PING_PONG_ANIMATION_H
#define PING_PONG_ANIMATION_H

#include <cstdint>
#include <cstdlib>

#include "animation.h"
#include "easing.h"

// A single white dot with a fading trail on both sides bounces back and
// forth between the center of the cross (pixel index kLedsPerStrip - 1) and
// the tip of every branch (pixel index 0), synced across all 4 branches.
// Each leg of the bounce follows the same S-curve speed profile as
// ParticleAnimation, so it eases to a smooth stop at each end before
// reversing.
class PingPongAnimation : public Animation {
public:
	const char *get_name() const override { return "Ping Pong"; }

	void start() override {
		age_ = 0.0f;
		moving_to_tip_ = true;
	}

	void update(float dt) override {
		age_ += dt;
		if (age_ >= kLegDurationS) {
			age_ -= kLegDurationS;
			moving_to_tip_ = !moving_to_tip_;
		}
	}

	void render(LedStrip *strips[kNumStrips]) override {
		for (uint s = 0; s < kNumStrips; s++) {
			strips[s]->clear();
		}

		float s_leg = age_ / kLegDurationS; // 0..1 within the current leg
		float eased = ease_in_out(s_leg);
		float travel = static_cast<float>(kLedsPerStrip - 1) * eased;
		float position = moving_to_tip_ ? (kLedsPerStrip - 1) - travel : travel;

		int head = static_cast<int>(position);

		for (int t = -kTailLength; t <= kTailLength; t++) {
			int idx = head + t;
			if (idx < 0 || idx >= static_cast<int>(kLedsPerStrip)) continue;

			float fade = 1.0f - static_cast<float>(std::abs(t)) / kTailLength;
			fade *= fade; // ease-out tail on both sides
			uint8_t level = static_cast<uint8_t>(255.0f * fade);

			for (uint s = 0; s < kNumStrips; s++) {
				strips[s]->set_pixel(idx, level, level, level);
			}
		}
	}

private:
	static constexpr float kLegDurationS = 1.1f; // seconds for one center<->tip leg
	static constexpr int kTailLength = 15;       // pixels of fading trail on each side

	float age_ = 0.0f;
	bool moving_to_tip_ = true;
};

#endif
