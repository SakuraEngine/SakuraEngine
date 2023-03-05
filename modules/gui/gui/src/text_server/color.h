/*************************************************************************/
/*  color.h                                                              */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once
#include "config.h"
namespace godot{
struct Color {
	union {
		struct {
			float r;
			float g;
			float b;
			float a;
		};
		float components[4] = { 0, 0, 0, 1.0 };
	};
	Color operator*(const Color &p_color) const;
	bool operator==(const Color &p_color) const;
	bool operator!=(const Color &p_color) const { return !(*this == p_color); }

    _FORCE_INLINE_ Color() {}

	/**
	 * RGBA construct parameters.
	 * Alpha is not optional as otherwise we can't bind the RGB version for scripting.
	 */
	_FORCE_INLINE_ Color(float p_r, float p_g, float p_b, float p_a) {
		r = p_r;
		g = p_g;
		b = p_b;
		a = p_a;
	}

	/**
	 * RGB construct parameters.
	 */
	_FORCE_INLINE_ Color(float p_r, float p_g, float p_b) {
		r = p_r;
		g = p_g;
		b = p_b;
		a = 1.0;
	}

	/**
	 * Construct a Color from another Color, but with the specified alpha value.
	 */
	_FORCE_INLINE_ Color(const Color &p_c, float p_a) {
		r = p_c.r;
		g = p_c.g;
		b = p_c.b;
		a = p_a;
	}
};

_FORCE_INLINE_ Color Color::operator*(const Color &p_color) const {
	return Color(
			r * p_color.r,
			g * p_color.g,
			b * p_color.b,
			a * p_color.a);
}
_FORCE_INLINE_ bool Color::operator==(const Color &p_color) const {
	return (
			r == p_color.r&&
			g == p_color.g&&
			b == p_color.b&&
			a == p_color.a);
}
}