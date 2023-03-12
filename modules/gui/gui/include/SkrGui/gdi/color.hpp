#pragma once
#include "utils/types.h"
#include <cmath>

namespace skr {
namespace gdi {

inline static float modf(float a, float b) { return fmodf(a, b); }
inline static float clampf(float a, float mn, float mx) { return a < mn ? mn : (a > mx ? mx : a); }

inline static float hue(float h, float m1, float m2)
{
	if (h < 0) h += 1;
	if (h > 1) h -= 1;
	if (h < 1.0f/6.0f)
		return m1 + (m2 - m1) * h * 6.0f;
	else if (h < 3.0f/6.0f)
		return m2;
	else if (h < 4.0f/6.0f)
		return m1 + (m2 - m1) * (2.0f/3.0f - h) * 6.0f;
	return m1;
}

inline static skr_float4_t hsla_to_rgbaf(float h, float s, float l, uint8_t a)
{
	float m1, m2;
	skr_float4_t col;
	h = modf(h, 1.0f);
	if (h < 0.0f) h += 1.0f;
	s = clampf(s, 0.0f, 1.0f);
	l = clampf(l, 0.0f, 1.0f);
	m2 = l <= 0.5f ? (l * (1 + s)) : (l + s - l * s);
	m1 = 2 * l - m2;
	col.x = clampf(hue(h + 1.0f/3.0f, m1, m2), 0.0f, 1.0f);
	col.y = clampf(hue(h, m1, m2), 0.0f, 1.0f);
	col.z = clampf(hue(h - 1.0f/3.0f, m1, m2), 0.0f, 1.0f);
	col.w = a / 255.0f;
	return col;
}

inline static skr_float4_t hsl_to_rgbaf(float h, float s, float l)
{
	return hsla_to_rgbaf(h, s, l, 255);
}

} }