/*************************************************************************/
/*  vector3.h                                                            */
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
struct Vector3i {
	enum Axis {
		AXIS_X,
		AXIS_Y,
		AXIS_Z,
	};

	union {
		struct {
			int32_t x;
			int32_t y;
			int32_t z;
		};

		int32_t coord[3] = { 0 };
	};
	
	_FORCE_INLINE_ Vector3i() {}
	_FORCE_INLINE_ Vector3i(const int32_t p_x, const int32_t p_y, const int32_t p_z) {
		x = p_x;
		y = p_y;
		z = p_z;
	}
};

struct Vector3 {
	static const int AXIS_COUNT = 3;

	enum Axis {
		AXIS_X,
		AXIS_Y,
		AXIS_Z,
	};

	union {
		struct {
			real_t x;
			real_t y;
			real_t z;
		};

		real_t coord[3] = { 0 };
	};

	_FORCE_INLINE_ operator Vector3i() const {
		return Vector3i(static_cast<int32_t>(x), static_cast<int32_t>(y), static_cast<int32_t>(z));
	}

	_FORCE_INLINE_ Vector3() {}
	_FORCE_INLINE_ Vector3(const Vector3i &p_ivec) {
		x = static_cast<real_t>(p_ivec.x);
		y = static_cast<real_t>(p_ivec.y);
		z = static_cast<real_t>(p_ivec.z);
	}
	_FORCE_INLINE_ Vector3(const real_t p_x, const real_t p_y, const real_t p_z) {
		x = p_x;
		y = p_y;
		z = p_z;
	}
};
}