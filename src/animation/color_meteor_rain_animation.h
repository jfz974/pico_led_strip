#ifndef COLOR_METEOR_RAIN_ANIMATION_H
#define COLOR_METEOR_RAIN_ANIMATION_H

#include <cstdint>
#include <cstdlib>

#include "animation.h"
#include "color.h"
#include "pico/time.h"

// A variant of MeteorRainAnimation: each meteor gets its own random hue
// instead of white, speed varies over a wider spread, and meteors spawn
// faster / more of them are allowed in flight at once for a denser rain.
class ColorMeteorRainAnimation : public Animation {
public:
	const char *get_name() const override { return "Color Meteor Rain"; }

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
				float level = m.brightness * fade;

				uint8_t r = static_cast<uint8_t>(m.r * level);
				uint8_t g = static_cast<uint8_t>(m.g * level);
				uint8_t b = static_cast<uint8_t>(m.b * level);
				strips[m.strip_index]->set_pixel(idx, r, g, b);
			}
		}
	}

private:
	struct Meteor {
		uint8_t strip_index = 0;
		float position = 0.0f;
		float speed = 0.0f;
		float brightness = 1.0f; // 0..1 multiplier on top of the tail fade
		uint8_t r = 255, g = 255, b = 255;
		bool active = false;
	};

	static constexpr int kTailLength = 12;
	static constexpr uint kMaxMeteors = 20;       // more concurrent meteors than MeteorRainAnimation
	static constexpr float kMinIntervalS = 0.08f; // spawn noticeably more often
	static constexpr float kMaxIntervalS = 0.3f;
	static constexpr float kMinSpeed = 120.0f;    // wider speed spread
	static constexpr float kMaxSpeed = 550.0f;

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
				m.brightness = 0.6f + 0.4f * random01();
				hsv_to_rgb(random01(), m.r, m.g, m.b);
				return;
			}
		}
	}

	Meteor meteors_[kMaxMeteors];
	float next_spawn_s_[kNumStrips] = {0};
};

#endif
