#ifndef HEARTBEAT_ANIMATION_H
#define HEARTBEAT_ANIMATION_H

#include <cmath>
#include <cstdint>

#include "animation.h"

// The whole cross brightens and dims together in a "lub-dub" heartbeat
// pattern: two quick pulses (the second softer and slightly wider than the
// first) followed by a rest, repeating every kPeriodS seconds.
class HeartbeatAnimation : public Animation {
public:
	const char *get_name() const override { return "Heartbeat"; }

	void start() override {
		time_ = 0.0f;
	}

	void update(float dt) override {
		time_ += dt;
		if (time_ >= kPeriodS) {
			time_ -= kPeriodS;
		}
	}

	void render(LedStrip *strips[kNumStrips]) override {
		uint8_t level = static_cast<uint8_t>(255.0f * brightness(time_));

		for (uint s = 0; s < kNumStrips; s++) {
			for (uint i = 0; i < kLedsPerStrip; i++) {
				strips[s]->set_pixel(i, level, level, level);
			}
		}
	}

private:
	static constexpr float kPeriodS = 1.2f;
	static constexpr float kIdleGlow = 0.03f; // faint glow between beats

	// A smooth raised-cosine bump: 1 at `center`, tapering to 0 by
	// `half_width` on either side, scaled by `amplitude`.
	static float bump(float t, float center, float half_width, float amplitude) {
		float d = t - center;
		if (d < -half_width || d > half_width) return 0.0f;
		float shape = cosf(d / half_width * 1.5707963f); // pi/2
		return amplitude * shape * shape;
	}

	static float brightness(float t) {
		float lub = bump(t, 0.10f, 0.09f, 1.0f);
		float dub = bump(t, 0.34f, 0.12f, 0.7f);
		float level = kIdleGlow + (lub > dub ? lub : dub);
		return level > 1.0f ? 1.0f : level;
	}

	float time_ = 0.0f;
};

#endif
