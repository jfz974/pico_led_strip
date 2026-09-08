#ifndef EASING_H
#define EASING_H

// Smoothstep: 0 at s=0, 1 at s=1, zero slope (zero speed) at both ends and
// maximum slope (peak speed) at the midpoint -- the classic S-curve motion
// profile. Expects s clamped to [0, 1] by the caller.
inline float ease_in_out(float s) {
	return s * s * (3.0f - 2.0f * s);
}

#endif
