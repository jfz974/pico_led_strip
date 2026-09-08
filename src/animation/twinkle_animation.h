#ifndef TWINKLE_ANIMATION_H
#define TWINKLE_ANIMATION_H

#include <cstdint>
#include <cstdlib>

#include "animation.h"
#include "pico/time.h"

// Confetti-style twinkle: random pixels across all 4 branches flash on at a
// warm-white sparkle color and fade back out, overlaid on a dark
// background.
class TwinkleAnimation : public Animation {
public:
	const char *get_name() const override { return "Twinkle"; }

	void start() override {
		srand(static_cast<unsigned>(time_us_32()));
		for (auto &t : twinkles_) {
			t.active = false;
		}
	}

	void update(float dt) override {
		for (auto &t : twinkles_) {
			if (t.active) {
				t.age += dt;
				if (t.age >= kLifetimeS) {
					t.active = false;
				}
			} else if (random01() < kSpawnChancePerSecond * dt) {
				spawn(t);
			}
		}
	}

	void render(LedStrip *strips[kNumStrips]) override {
		for (uint s = 0; s < kNumStrips; s++) {
			strips[s]->clear();
		}

		for (auto &t : twinkles_) {
			if (!t.active) continue;

			float fade = 1.0f - t.age / kLifetimeS;
			fade *= fade;

			uint8_t r = static_cast<uint8_t>(t.r * fade);
			uint8_t g = static_cast<uint8_t>(t.g * fade);
			uint8_t b = static_cast<uint8_t>(t.b * fade);
			strips[t.strip_index]->set_pixel(t.pixel_index, r, g, b);
		}
	}

private:
	struct Twinkle {
		uint8_t strip_index = 0;
		uint16_t pixel_index = 0;
		uint8_t r = 255, g = 255, b = 255;
		float age = 0.0f;
		bool active = false;
	};

	static constexpr uint kMaxTwinkles = 40;
	static constexpr float kLifetimeS = 0.6f;
	static constexpr float kSpawnChancePerSecond = 32.0f; // expected spawns/sec while slots free

	static float random01() {
		return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	}

	static uint8_t random_u8_range(uint8_t lo, uint8_t hi) {
		return static_cast<uint8_t>(lo + (rand() % (hi - lo + 1)));
	}

	static void spawn(Twinkle &t) {
		t.active = true;
		t.age = 0.0f;
		t.strip_index = static_cast<uint8_t>(rand() % kNumStrips);
		t.pixel_index = static_cast<uint16_t>(rand() % kLedsPerStrip);
		t.r = 255;
		t.g = random_u8_range(200, 255);
		t.b = random_u8_range(160, 220);
	}

	Twinkle twinkles_[kMaxTwinkles];
};

#endif
