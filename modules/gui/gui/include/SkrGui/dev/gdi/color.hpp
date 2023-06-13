#pragma once
#include "misc/types.h"
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

inline static uint32_t hsv_to_abgr(double H, double S, double V) {
	double r = 0, g = 0, b = 0;
	if (S == 0)
	{
		r = V;
		g = V;
		b = V;
	}
	else
	{
		int i;
		double f, p, q, t;

		if (H == 360)
			H = 0;
		else
			H = H / 60;

		i = (int)trunc(H);
		f = H - i;

		p = V * (1.0 - S);
		q = V * (1.0 - (S * f));
		t = V * (1.0 - (S * (1.0 - f)));

		switch (i)
		{
		case 0:
			r = V;
			g = t;
			b = p;
			break;

		case 1:
			r = q;
			g = V;
			b = p;
			break;

		case 2:
			r = p;
			g = V;
			b = t;
			break;

		case 3:
			r = p;
			g = q;
			b = V;
			break;

		case 4:
			r = t;
			g = p;
			b = V;
			break;

		default:
			r = V;
			g = p;
			b = q;
			break;
		}

	}
	const uint32_t R = (uint32_t)(r * 255.0);
	const uint32_t G = (uint32_t)(g * 255.0);
	const uint32_t B = (uint32_t)(b * 255.0);
	const uint32_t A = 255u;
	return (A << 24) | (B << 16) | (G << 8) | R;
}

inline static uint32_t encode_rgba(float r, float g, float b, float a)
{
	const uint32_t R = (uint32_t)(r * 255.0);
	const uint32_t G = (uint32_t)(g * 255.0);
	const uint32_t B = (uint32_t)(b * 255.0);
	const uint32_t A = (uint32_t)(a * 255.0);
	return (A << 24) | (B << 16) | (G << 8) | R;
}

} }