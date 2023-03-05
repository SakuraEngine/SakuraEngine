/*************************************************************************/
/*  vector2.h                                                            */
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
#include <cmath>

namespace godot{
struct Vector2 {
	static const int AXIS_COUNT = 2;

	enum Axis {
		AXIS_X,
		AXIS_Y,
	};

	union {
		struct {
			union {
				real_t x;
				real_t width;
			};
			union {
				real_t y;
				real_t height;
			};
		};

		real_t coord[2] = { 0 };
	};

	void normalize()
	{
		real_t l = x * x + y * y;
		if (l != 0) {
			l = ::sqrt(l);
			x /= l;
			y /= l;
		}
	}

	Vector2 normalized() const
	{
		Vector2 v = *this;
		v.normalize();
		return v;
	}

	Vector2 round() const
	{
		return Vector2(std::round(x), std::round(y));
	}

	real_t dot(const Vector2 &p_other) const 
	{
		return x * p_other.x + y * p_other.y;
	}

	real_t cross(const Vector2 &p_other) const 
	{
		return x * p_other.y - y * p_other.x;
	}

	real_t length() const
	{
		return ::sqrt(x * x + y * y);
	}

	Vector2 rotated(const real_t p_by) const {
		real_t sine = ::sin(p_by);
		real_t cosi = ::cos(p_by);
		return Vector2(
				x * cosi - y * sine,
				x * sine + y * cosi);
	}

	real_t& operator[](int p_axis) { return coord[p_axis]; }
	real_t operator[](int p_axis) const { return coord[p_axis]; }

	Vector2 operator+(const Vector2 &p_v) const;
	void operator+=(const Vector2 &p_v);

	Vector2 operator-() const { return Vector2(-x, -y); }
	
	Vector2 operator-(const Vector2 &p_v) const;
	void operator-=(const Vector2 &p_v);

	Vector2 operator/(const Vector2 &p_v1) const;
	Vector2 operator/(const real_t &rvalue) const;

	Vector2 operator*(const real_t &rvalue) const;
	Vector2 operator*(const Vector2 &p_v1) const;
	Vector2 operator*= (const real_t &rvalue) { return *this = *this * rvalue; }
	Vector2 operator*= (const Vector2 &p_v1) { return *this = *this * p_v1; }
	
	bool operator==(const Vector2 &p_vec2) const;
	bool operator!=(const Vector2 &p_vec2) const;

	bool operator<(const Vector2 &p_vec2) const { return x == p_vec2.x ? (y < p_vec2.y) : (x < p_vec2.x); }
	bool operator>(const Vector2 &p_vec2) const { return x == p_vec2.x ? (y > p_vec2.y) : (x > p_vec2.x); }
	bool operator<=(const Vector2 &p_vec2) const { return x == p_vec2.x ? (y <= p_vec2.y) : (x < p_vec2.x); }
	bool operator>=(const Vector2 &p_vec2) const { return x == p_vec2.x ? (y >= p_vec2.y) : (x > p_vec2.x); }
	void operator/=(const Vector2 &rvalue) { *this = *this / rvalue; }

	_FORCE_INLINE_ Vector2() {}
	_FORCE_INLINE_ Vector2(const real_t p_x, const real_t p_y) {
		x = p_x;
		y = p_y;
	}
};
_FORCE_INLINE_ Vector2 Vector2::operator/(const Vector2 &p_v1) const {
	return Vector2(x / p_v1.x, y / p_v1.y);
}

_FORCE_INLINE_ Vector2 Vector2::operator*(const Vector2 &p_v1) const {
	return Vector2(x * p_v1.x, y * p_v1.y);
}

_FORCE_INLINE_ Vector2 operator*(const float p_scalar, const Vector2 &p_vec) {
	return p_vec * p_scalar;
}

_FORCE_INLINE_ Vector2 Vector2::operator+(const Vector2 &p_v) const {
	return Vector2(x + p_v.x, y + p_v.y);
}

_FORCE_INLINE_ Vector2 Vector2::operator-(const Vector2 &p_v) const {
	return Vector2(x - p_v.x, y - p_v.y);
}

_FORCE_INLINE_ Vector2 Vector2::operator*(const real_t &rvalue) const {
	return Vector2(x * rvalue, y * rvalue);
}

_FORCE_INLINE_ void Vector2::operator+=(const Vector2 &p_v) {
	x += p_v.x;
	y += p_v.y;
}

_FORCE_INLINE_ void Vector2::operator-=(const Vector2 &p_v) {
	x -= p_v.x;
	y -= p_v.y;
}

_FORCE_INLINE_ Vector2 Vector2::operator/(const real_t &rvalue) const {
	return Vector2(x / rvalue, y / rvalue);
}

_FORCE_INLINE_ bool Vector2::operator==(const Vector2 &p_vec2) const {
	return x == p_vec2.x && y == p_vec2.y;
}

_FORCE_INLINE_ bool Vector2::operator!=(const Vector2 &p_vec2) const {
	return x != p_vec2.x || y != p_vec2.y;
}

using Size2 = Vector2;
using Point2 = Vector2;

struct Vector2i {
	enum Axis {
		AXIS_X,
		AXIS_Y,
	};

	union {
		int32_t x = 0;
		int32_t width;
	};
	union {
		int32_t y = 0;
		int32_t height;
	};

	operator Vector2() const { return Vector2(static_cast<real_t>(x), static_cast<real_t>(y)); }
	void operator+=(const Vector2i &p_v){
		x += p_v.x;
		y += p_v.y;
	}
	bool operator<(const Vector2i &p_vec2) const { return (x == p_vec2.x) ? (y < p_vec2.y) : (x < p_vec2.x); }
	bool operator>(const Vector2i &p_vec2) const { return (x == p_vec2.x) ? (y > p_vec2.y) : (x > p_vec2.x); }

	bool operator<=(const Vector2i &p_vec2) const { return x == p_vec2.x ? (y <= p_vec2.y) : (x < p_vec2.x); }
	bool operator>=(const Vector2i &p_vec2) const { return x == p_vec2.x ? (y >= p_vec2.y) : (x > p_vec2.x); }

	bool operator==(const Vector2i &p_vec2) const;
	bool operator!=(const Vector2i &p_vec2) const;

	inline Vector2i() {}
	inline Vector2i(const Vector2 &p_vec2) {
		x = (int32_t)p_vec2.x;
		y = (int32_t)p_vec2.y;
	}
	inline Vector2i(const int32_t p_x, const int32_t p_y) {
		x = p_x;
		y = p_y;
	}
};

_FORCE_INLINE_ bool Vector2i::operator==(const Vector2i &p_vec2) const {
	return x == p_vec2.x && y == p_vec2.y;
}

_FORCE_INLINE_ bool Vector2i::operator!=(const Vector2i &p_vec2) const {
	return x != p_vec2.x || y != p_vec2.y;
}

typedef Vector2i Size2i;
typedef Vector2i Point2i;
}