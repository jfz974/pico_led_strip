#ifndef METEOR_RAIN_ANIMATION_H
#define METEOR_RAIN_ANIMATION_H

#include <cstdint>
#include <cstdlib>

#include "animation.h"
#include "pico/time.h"

// Unlike ParticleAnimation (one synced comet thrown down every branch at
// once, on a fixed interval), this throws meteors independently per branch
// at random intervals, speeds, and brightness -- a more organic, ongoing
// "rain" rather than a single radiating pulse. Each meteor travels at a
// constant speed (no S-curve) from the center (kLedsPerStrip - 1) out to
// the tip (0).
class MeteorRainAnimation : public Animation {
public:
	const char *get_name() const override { return "Meteor Rain"; }

	void start() override {
		srand(static_cast<unsigned>(time_us_32()));
		for (auto &m : meteors_) {
			m.active = false;
		}
		for (uint s = 0; s < kNumStrips; s++) {
			next_spawn_s_[s] = random_interval();
		}
	}

	void update(float dt) override {
		for (uint s = 0; s < kNumStrips; s++) {
			next_spawn_s_[s] -= dt;
			if (next_spawn_s_[s] <= 0.0f) {
				spawn_meteor(s);
				next_spawn_s_[s] = random_interval();
			}
		}

		for (auto &m : meteors_) {
			if (!m.active) continue;
			m.position -= m.speed * dt;
			if (m.position + kTailLength < 0.0f) {
				m.active = false;
			}
		}
	}

	void render(LedStrip *strips[kNumStrips]) override {
		for (uint s = 0; s < kNumStrips; s++) {
			strips[s]->clear();
		}

		for (auto &m : meteors_) {
			if (!m.active) continue;

			int head = static_cast<int>(m.position);
			for (int t = 0; t <= kTailLength; t++) {
				int idx = head + t; // tail trails toward the center
				if (idx < 0 || idx >= static_cast<int>(kLedsPerStrip)) continue;

				float fade = 1.0f - static_cast<float>(t) / kTailLength;
				fade *= fade;
				uint8_t level = static_cast<uint8_t>(m.brightness * fade);
				strips[m.strip_index]->set_pixel(idx, level, level, level);
			}
		}
	}

private:
	struct Meteor {
		uint8_t strip_index = 0;
		float position = 0.0f;
		float speed = 0.0f;
		float brightness = 255.0f;
		bool active = false;
	};

	static constexpr int kTailLength = 12;
	static constexpr uint kMaxMeteors = 12;
	static constexpr float kMinIntervalS = 0.15f;
	static constexpr float kMaxIntervalS = 0.5f;
	static constexpr float kMinSpeed = 200.0f;
	static constexpr float kMaxSpeed = 400.0f;

	static float random01() {
		return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	}

	static float random_interval() {
		return kMinIntervalS + (kMaxIntervalS - kMinIntervalS) * random01();
	}

	void spawn_meteor(uint strip_index) {
		for (auto &m : meteors_) {
			if (!m.active) {
				m.active = true;
				m.strip_index = static_cast<uint8_t>(strip_index);
				m.position = static_cast<float>(kLedsPerStrip - 1);
				m.speed = kMinSpeed + (kMaxSpeed - kMinSpeed) * random01();
				m.brightness = 160.0f + 95.0f * random01();
				return;
			}
		}
	}

	Meteor meteors_[kMaxMeteors];
	float next_spawn_s_[kNumStrips] = {0};
};

#endif
