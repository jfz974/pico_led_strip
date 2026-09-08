#ifndef FAST_TWINKLE_ANIMATION_H
#define FAST_TWINKLE_ANIMATION_H

#include <cstdint>
#include <cstdlib>

#include "animation.h"
#include "pico/time.h"

// A denser, faster twinkle than TwinkleAnimation: up to 50 independent
// white points scattered across all 4 branches, each igniting at a random,
// unsynchronized moment -- every currently dark slot has a 5% chance each
// frame to start a new point. Each point does a quick attack (3-4 frames,
// 0% -> 100%) followed by a slower release (6-8 frames, 100% -> 0%), both
// randomly chosen per point and counted in actual update() calls rather
// than wall-clock time, so the envelope shape doesn't depend on the current
// frame rate. Each active point's immediate left/right neighbor pixels glow
// at 20% of its intensity. A new point is only placed at least kMinSpacing
// pixels away (on the same branch) from every other currently active
// point, so one point's footprint (pixel-1, pixel, pixel+1) never overlaps
// another's.
class FastTwinkleAnimation : public Animation {
public:
	const char *get_name() const override { return "Fast Twinkle"; }

	void start() override {
		srand(static_cast<unsigned>(time_us_32()));
		for (auto &p : points_) {
			p.active = false;
		}
	}

	void update(float /*dt*/) override {
		for (auto &p : points_) {
			if (!p.active) continue;
			p.frame_age++;
			if (p.frame_age >= p.attack_frames + p.release_frames) {
				p.active = false;
			}
		}

		for (auto &p : points_) {
			if (p.active) continue;
			if (random01() < kSpawnChancePerFrame) {
				try_spawn(p);
			}
		}
	}

	void render(LedStrip *strips[kNumStrips]) override {
		for (uint s = 0; s < kNumStrips; s++) {
			for (uint i = 0; i < kLedsPerStrip; i++) {
				intensity_[s][i] = 0.0f;
			}
		}

		for (auto &p : points_) {
			if (!p.active) continue;

			float base = point_intensity(p);
			accumulate(p.strip_index, p.pixel_index, base);
			if (p.pixel_index > 0) {
				accumulate(p.strip_index, p.pixel_index - 1, base * kNeighborFraction);
			}
			if (p.pixel_index + 1 < kLedsPerStrip) {
				accumulate(p.strip_index, p.pixel_index + 1, base * kNeighborFraction);
			}
		}

		for (uint s = 0; s < kNumStrips; s++) {
			for (uint i = 0; i < kLedsPerStrip; i++) {
				uint8_t level = static_cast<uint8_t>(255.0f * intensity_[s][i]);
				strips[s]->set_pixel(i, level, level, level);
			}
		}
	}

private:
	struct Point {
		bool active = false;
		uint8_t strip_index = 0;
		uint16_t pixel_index = 0;
		int frame_age = 0;
		int attack_frames = 3;
		int release_frames = 6;
	};

	static constexpr uint kMaxPoints = 50;
	static constexpr float kSpawnChancePerFrame = 0.05f; // a dark spot's chance to start, per frame
	static constexpr int kMinSpacing = 3;                // min pixels between active points on a branch
	static constexpr float kNeighborFraction = 0.20f;

	static float random01() {
		return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	}

	static int random_range(int lo, int hi) { // inclusive
		return lo + static_cast<int>(rand() % (hi - lo + 1));
	}

	static float point_intensity(const Point &p) {
		if (p.frame_age < p.attack_frames) {
			return static_cast<float>(p.frame_age + 1) / static_cast<float>(p.attack_frames);
		}
		int release_age = p.frame_age - p.attack_frames;
		float level = 1.0f - static_cast<float>(release_age + 1) / static_cast<float>(p.release_frames);
		return level < 0.0f ? 0.0f : level;
	}

	void accumulate(uint strip_index, uint pixel_index, float value) {
		float &slot = intensity_[strip_index][pixel_index];
		if (value > slot) {
			slot = value;
		}
	}

	bool too_close(uint strip_index, uint16_t pixel_index) const {
		for (auto &p : points_) {
			if (!p.active || p.strip_index != strip_index) continue;
			int d = static_cast<int>(p.pixel_index) - static_cast<int>(pixel_index);
			if (d < 0) d = -d;
			if (d < kMinSpacing) return true;
		}
		return false;
	}

	void try_spawn(Point &p) {
		uint strip_index = static_cast<uint>(rand() % kNumStrips);
		uint16_t pixel_index = static_cast<uint16_t>(rand() % kLedsPerStrip);
		if (too_close(strip_index, pixel_index)) return; // crowded; try again another frame

		p.active = true;
		p.strip_index = static_cast<uint8_t>(strip_index);
		p.pixel_index = pixel_index;
		p.frame_age = 0;
		p.attack_frames = random_range(3, 4);
		p.release_frames = random_range(6, 8);
	}

	Point points_[kMaxPoints];
	float intensity_[kNumStrips][kLedsPerStrip] = {};
};

#endif
