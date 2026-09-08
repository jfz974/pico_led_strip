#ifndef COLOR_H
#define COLOR_H

#include <cstdint>

// Full-saturation, full-value HSV to RGB (h in [0,1)).
inline void hsv_to_rgb(float h, uint8_t &r, uint8_t &g, uint8_t &b) {
	float h6 = h * 6.0f;
	int i = static_cast<int>(h6);
	float f = h6 - static_cast<float>(i);
	float q = 1.0f - f;
	float t = f;

	float rf, gf, bf;
	switch (i % 6) {
		case 0: rf = 1.0f; gf = t; bf = 0.0f; break;
		case 1: rf = q; gf = 1.0f; bf = 0.0f; break;
		case 2: rf = 0.0f; gf = 1.0f; bf = t; break;
		case 3: rf = 0.0f; gf = q; bf = 1.0f; break;
		case 4: rf = t; gf = 0.0f; bf = 1.0f; break;
		default: rf = 1.0f; gf = 0.0f; bf = q; break;
	}

	r = static_cast<uint8_t>(rf * 255.0f);
	g = static_cast<uint8_t>(gf * 255.0f);
	b = static_cast<uint8_t>(bf * 255.0f);
}

#endif
