#ifndef LARSON_SCANNER_ANIMATION_H
#define LARSON_SCANNER_ANIMATION_H

#include <cstdint>
#include <cstdlib>

#include "animation.h"

// The classic Knight Rider / Cylon eye: a red dot with a short fading tail
// scanning back and forth at a constant speed (deliberately linear, no
// easing -- unlike PingPongAnimation's smooth S-curve bounce, this is the
// snappier, constant-speed original).
class LarsonScannerAnimation : public Animation {
public:
	const char *get_name() const override { return "Larson Scanner"; }

	void start() override {
		position_ = 0.0f;
		direction_ = 1.0f;
	}

	void update(float dt) override {
		position_ += direction_ * kSpeed * dt;
		if (position_ <= 0.0f) {
			position_ = 0.0f;
			direction_ = 1.0f;
		} else if (position_ >= static_cast<float>(kLedsPerStrip - 1)) {
			position_ = static_cast<float>(kLedsPerStrip - 1);
			direction_ = -1.0f;
		}
	}

	void render(LedStrip *strips[kNumStrips]) override {
		for (uint s = 0; s < kNumStrips; s++) {
			strips[s]->clear();
		}

		int head = static_cast<int>(position_);
		for (int t = -kTailLength; t <= kTailLength; t++) {
			int idx = head + t;
			if (idx < 0 || idx >= static_cast<int>(kLedsPerStrip)) continue;

			float fade = 1.0f - static_cast<float>(std::abs(t)) / kTailLength;
			fade *= fade;
			uint8_t level = static_cast<uint8_t>(255.0f * fade);

			for (uint s = 0; s < kNumStrips; s++) {
				strips[s]->set_pixel(idx, level, 0, 0); // red eye
			}
		}
	}

private:
	static constexpr float kSpeed = 500.0f; // LEDs per second, constant
	static constexpr int kTailLength = 10;

	float position_ = 0.0f;
	float direction_ = 1.0f;
};

#endif
