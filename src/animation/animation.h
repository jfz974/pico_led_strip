#ifndef ANIMATION_H
#define ANIMATION_H

#include "ws2812_strip.h"

// Shared LED strip configuration -- every animation targets the same
// physical layout: 4 branches of a diagonal cross, 400 LEDs each.
constexpr uint kNumStrips = 4;
constexpr uint kLedsPerStrip = 400;

using LedStrip = Ws2812Strip<kLedsPerStrip>;

// Common interface every animation implements, so main() can drive (and
// later switch between) animations without knowing their concrete type.
class Animation {
public:
	virtual ~Animation() = default;

	// Short human-readable name, e.g. for logging which animation is active.
	virtual const char *get_name() const = 0;

	// Called once when the animation becomes active; (re)sets its state.
	virtual void start() {}

	// Advances the animation's internal state by dt seconds.
	virtual void update(float dt) = 0;

	// Writes the current frame's pixel data to every strip. Does not call
	// show() -- the caller is responsible for pushing the frame out.
	virtual void render(LedStrip *strips[kNumStrips]) = 0;
};

#endif
