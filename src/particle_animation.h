#ifndef PARTICLE_ANIMATION_H
#define PARTICLE_ANIMATION_H

#include <cstdint>

#include "animation.h"
#include "easing.h"

// Throws a white comet-style particle from the shared center of the cross
// (pixel index kLedsPerStrip - 1, the far end from DIN) out to the outer
// tip of every branch at once (pixel index 0), spawning a new particle
// every kSpawnIntervalS seconds. Each particle follows an S-curve speed
// profile: it eases in from a stop at the center, reaches peak speed around
// the midpoint of the branch, then eases back down to a stop right at the
// tip, instead of moving at a constant speed.
class ParticleAnimation : public Animation {
public:
	void start() override {
		for (auto &p : particles_) {
			p.active = false;
		}
		time_since_spawn_ = kSpawnIntervalS; // spawn one immediately
	}

	void update(float dt) override {
		time_since_spawn_ += dt;
		if (time_since_spawn_ >= kSpawnIntervalS) {
			time_since_spawn_ -= kSpawnIntervalS;
			spawn_particle();
		}

		for (auto &p : particles_) {
			if (!p.active) continue;
			p.age += dt;
			if (p.age >= kTravelDurationS) {
				p.active = false;
			}
		}
	}

	void render(LedStrip *strips[kNumStrips]) override {
		for (uint i = 0; i < kNumStrips; i++) {
			strips[i]->clear();
		}

		for (auto &p : particles_) {
			if (!p.active) continue;

			float s = p.age / kTravelDurationS; // 0 at spawn, 1 at the tip
			float eased = ease_in_out(s);
			float position = static_cast<float>(kLedsPerStrip - 1) * (1.0f - eased);
			int head = static_cast<int>(position);

			for (int t = 0; t <= kTailLength; t++) {
				int idx = head + t; // tail trails toward the center (higher index)
				if (idx < 0 || idx >= static_cast<int>(kLedsPerStrip)) continue;

				float fade = 1.0f - static_cast<float>(t) / kTailLength;
				fade *= fade; // ease-out tail
				uint8_t level = static_cast<uint8_t>(255.0f * fade);

				for (uint i = 0; i < kNumStrips; i++) {
					strips[i]->set_pixel(idx, level, level, level);
				}
			}
		}
	}

private:
	static constexpr float kSpawnIntervalS = 0.5f;
	static constexpr float kTravelDurationS = 1.3f; // seconds to cross the whole branch
	static constexpr int kTailLength = 20;          // pixels of fading tail behind the head
	static constexpr uint kMaxParticles = 8;        // headroom above kTravelDurationS / kSpawnIntervalS

	struct Particle {
		float age = 0.0f; // seconds since spawn
		bool active = false;
	};

	void spawn_particle() {
		for (auto &p : particles_) {
			if (!p.active) {
				p.active = true;
				p.age = 0.0f;
				return;
			}
		}
	}

	Particle particles_[kMaxParticles];
	float time_since_spawn_ = 0.0f;
};

#endif
