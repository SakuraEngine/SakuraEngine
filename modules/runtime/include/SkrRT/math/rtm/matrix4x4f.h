#pragma once

////////////////////////////////////////////////////////////////////////////////
// The MIT License (MIT)
//
// Copyright (c) 2018 Nicholas Frechette & Realtime Math contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include "SkrRT/math/rtm/macros.h"
#include "SkrRT/math/rtm/math.h"
#include "SkrRT/math/rtm/matrix3x3f.h"
#include "SkrRT/math/rtm/vector4f.h"
#include "SkrRT/math/rtm/version.h"
#include "SkrRT/math/rtm/impl/compiler_utils.h"
#include "SkrRT/math/rtm/impl/matrix_common.h"

RTM_IMPL_FILE_PRAGMA_PUSH

namespace rtm
{
	RTM_IMPL_VERSION_NAMESPACE_BEGIN

	//////////////////////////////////////////////////////////////////////////
	// Returns the desired 4x4 matrix axis.
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK RTM_FORCE_INLINE constexpr vector4f RTM_SIMD_CALL matrix_get_axis(matrix4x4f_arg0 input, axis4 axis) RTM_NO_EXCEPT
	{
		return axis == axis4::x ? input.x_axis : (axis == axis4::y ? input.y_axis : (axis == axis4::z ? input.z_axis : input.w_axis));
	}

	//////////////////////////////////////////////////////////////////////////
	// Multiplies two 4x4 matrices.
	// Multiplication order is as follow: local_to_world = matrix_mul(local_to_object, object_to_world)
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline matrix4x4f RTM_SIMD_CALL matrix_mul(matrix4x4f_arg0 lhs, matrix4x4f_arg1 rhs) RTM_NO_EXCEPT
	{
		vector4f tmp = vector_mul(vector_dup_x(lhs.x_axis), rhs.x_axis);
		tmp = vector_mul_add(vector_dup_y(lhs.x_axis), rhs.y_axis, tmp);
		tmp = vector_mul_add(vector_dup_z(lhs.x_axis), rhs.z_axis, tmp);
		vector4f x_axis = tmp;

		tmp = vector_mul(vector_dup_x(lhs.y_axis), rhs.x_axis);
		tmp = vector_mul_add(vector_dup_y(lhs.y_axis), rhs.y_axis, tmp);
		tmp = vector_mul_add(vector_dup_z(lhs.y_axis), rhs.z_axis, tmp);
		vector4f y_axis = tmp;

		tmp = vector_mul(vector_dup_x(lhs.z_axis), rhs.x_axis);
		tmp = vector_mul_add(vector_dup_y(lhs.z_axis), rhs.y_axis, tmp);
		tmp = vector_mul_add(vector_dup_z(lhs.z_axis), rhs.z_axis, tmp);
		vector4f z_axis = tmp;

		tmp = vector_mul(vector_dup_x(lhs.w_axis), rhs.x_axis);
		tmp = vector_mul_add(vector_dup_y(lhs.w_axis), rhs.y_axis, tmp);
		tmp = vector_mul_add(vector_dup_z(lhs.w_axis), rhs.z_axis, tmp);
		vector4f w_axis = vector_add(rhs.w_axis, tmp);
		return matrix4x4f{ x_axis, y_axis, z_axis, w_axis };
	}

	//////////////////////////////////////////////////////////////////////////
	// Multiplies a 4x4 matrix and a 4D vector.
	// Multiplication order is as follow: world_position = matrix_mul(local_position, local_to_world)
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK RTM_FORCE_INLINE vector4f RTM_SIMD_CALL matrix_mul_vector(vector4f_arg0 vec4, matrix4x4f_arg0 mtx) RTM_NO_EXCEPT
	{
		vector4f tmp;

		tmp = vector_mul(vector_dup_x(vec4), mtx.x_axis);
		tmp = vector_mul_add(vector_dup_y(vec4), mtx.y_axis, tmp);
		tmp = vector_mul_add(vector_dup_z(vec4), mtx.z_axis, tmp);
		tmp = vector_mul_add(vector_dup_w(vec4), mtx.w_axis, tmp);

		return tmp;
	}

	//////////////////////////////////////////////////////////////////////////
	// Transposes a 4x4 matrix.
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK RTM_FORCE_INLINE matrix4x4f RTM_SIMD_CALL matrix_transpose(matrix4x4f_arg0 input) RTM_NO_EXCEPT
	{
		vector4f x_axis;
		vector4f y_axis;
		vector4f z_axis;
		vector4f w_axis;
		RTM_MATRIXF_TRANSPOSE_4X4(input.x_axis, input.y_axis, input.z_axis, input.w_axis, x_axis, y_axis, z_axis, w_axis);
		return matrix4x4f{ x_axis, y_axis, z_axis, w_axis };
	}

	//////////////////////////////////////////////////////////////////////////
	// Inverses a 4x4 matrix.
	// If the input matrix is not invertible, the result is undefined.
	// For a safe alternative, supply a fallback value and a threshold.
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline matrix4x4f RTM_SIMD_CALL matrix_inverse(matrix4x4f_arg0 input) RTM_NO_EXCEPT
	{
		matrix4x4f input_transposed = matrix_transpose(input);

		vector4f v00 = vector_mix<mix4::x, mix4::x, mix4::y, mix4::y>(input_transposed.z_axis, input_transposed.z_axis);
		vector4f v01 = vector_mix<mix4::x, mix4::x, mix4::y, mix4::y>(input_transposed.x_axis, input_transposed.x_axis);
		vector4f v02 = vector_mix<mix4::x, mix4::z, mix4::a, mix4::c>(input_transposed.z_axis, input_transposed.x_axis);
		vector4f v10 = vector_mix<mix4::z, mix4::w, mix4::z, mix4::w>(input_transposed.w_axis, input_transposed.w_axis);
		vector4f v11 = vector_mix<mix4::z, mix4::w, mix4::z, mix4::w>(input_transposed.y_axis, input_transposed.y_axis);
		vector4f v12 = vector_mix<mix4::y, mix4::w, mix4::b, mix4::d>(input_transposed.w_axis, input_transposed.y_axis);

		vector4f d0 = vector_mul(v00, v10);
		vector4f d1 = vector_mul(v01, v11);
		vector4f d2 = vector_mul(v02, v12);

		v00 = vector_mix<mix4::z, mix4::w, mix4::z, mix4::w>(input_transposed.z_axis, input_transposed.z_axis);
		v01 = vector_mix<mix4::z, mix4::w, mix4::z, mix4::w>(input_transposed.x_axis, input_transposed.x_axis);
		v02 = vector_mix<mix4::y, mix4::w, mix4::b, mix4::d>(input_transposed.z_axis, input_transposed.x_axis);
		v10 = vector_mix<mix4::x, mix4::x, mix4::y, mix4::y>(input_transposed.w_axis, input_transposed.w_axis);
		v11 = vector_mix<mix4::x, mix4::x, mix4::y, mix4::y>(input_transposed.y_axis, input_transposed.y_axis);
		v12 = vector_mix<mix4::x, mix4::z, mix4::a, mix4::c>(input_transposed.w_axis, input_transposed.y_axis);

		d0 = vector_neg_mul_sub(v00, v10, d0);
		d1 = vector_neg_mul_sub(v01, v11, d1);
		d2 = vector_neg_mul_sub(v02, v12, d2);

		v00 = vector_mix<mix4::y, mix4::z, mix4::x, mix4::y>(input_transposed.y_axis, input_transposed.y_axis);
		v01 = vector_mix<mix4::z, mix4::x, mix4::y, mix4::x>(input_transposed.x_axis, input_transposed.x_axis);
		v02 = vector_mix<mix4::y, mix4::z, mix4::x, mix4::y>(input_transposed.w_axis, input_transposed.w_axis);
		vector4f v03 = vector_mix<mix4::z, mix4::x, mix4::y, mix4::x>(input_transposed.z_axis, input_transposed.z_axis);
		v10 = vector_mix<mix4::b, mix4::y, mix4::w, mix4::x>(d0, d2);
		v11 = vector_mix<mix4::w, mix4::b, mix4::y, mix4::z>(d0, d2);
		v12 = vector_mix<mix4::d, mix4::y, mix4::w, mix4::x>(d1, d2);
		vector4f v13 = vector_mix<mix4::w, mix4::d, mix4::y, mix4::z>(d1, d2);

		vector4f c0 = vector_mul(v00, v10);
		vector4f c2 = vector_mul(v01, v11);
		vector4f c4 = vector_mul(v02, v12);
		vector4f c6 = vector_mul(v03, v13);

		v00 = vector_mix<mix4::z, mix4::w, mix4::y, mix4::z>(input_transposed.y_axis, input_transposed.y_axis);
		v01 = vector_mix<mix4::w, mix4::z, mix4::w, mix4::y>(input_transposed.x_axis, input_transposed.x_axis);
		v02 = vector_mix<mix4::z, mix4::w, mix4::y, mix4::z>(input_transposed.w_axis, input_transposed.w_axis);
		v03 = vector_mix<mix4::w, mix4::z, mix4::w, mix4::y>(input_transposed.z_axis, input_transposed.z_axis);
		v10 = vector_mix<mix4::w, mix4::x, mix4::y, mix4::a>(d0, d2);
		v11 = vector_mix<mix4::z, mix4::y, mix4::a, mix4::x>(d0, d2);
		v12 = vector_mix<mix4::w, mix4::x, mix4::y, mix4::c>(d1, d2);
		v13 = vector_mix<mix4::z, mix4::y, mix4::c, mix4::x>(d1, d2);

		c0 = vector_neg_mul_sub(v00, v10, c0);
		c2 = vector_neg_mul_sub(v01, v11, c2);
		c4 = vector_neg_mul_sub(v02, v12, c4);
		c6 = vector_neg_mul_sub(v03, v13, c6);

		v00 = vector_mix<mix4::w, mix4::x, mix4::w, mix4::x>(input_transposed.y_axis, input_transposed.y_axis);
		v01 = vector_mix<mix4::y, mix4::w, mix4::x, mix4::z>(input_transposed.x_axis, input_transposed.x_axis);
		v02 = vector_mix<mix4::w, mix4::x, mix4::w, mix4::x>(input_transposed.w_axis, input_transposed.w_axis);
		v03 = vector_mix<mix4::y, mix4::w, mix4::x, mix4::z>(input_transposed.z_axis, input_transposed.z_axis);
		v10 = vector_mix<mix4::z, mix4::b, mix4::a, mix4::z>(d0, d2);
		v11 = vector_mix<mix4::b, mix4::x, mix4::w, mix4::a>(d0, d2);
		v12 = vector_mix<mix4::z, mix4::d, mix4::c, mix4::z>(d1, d2);
		v13 = vector_mix<mix4::d, mix4::x, mix4::w, mix4::c>(d1, d2);

		vector4f c1 = vector_neg_mul_sub(v00, v10, c0);
		c0 = vector_mul_add(v00, v10, c0);
		vector4f c3 = vector_mul_add(v01, v11, c2);
		c2 = vector_neg_mul_sub(v01, v11, c2);
		vector4f c5 = vector_neg_mul_sub(v02, v12, c4);
		c4 = vector_mul_add(v02, v12, c4);
		vector4f c7 = vector_mul_add(v03, v13, c6);
		c6 = vector_neg_mul_sub(v03, v13, c6);

		vector4f x_axis = vector_mix<mix4::x, mix4::b, mix4::z, mix4::d>(c0, c1);
		vector4f y_axis = vector_mix<mix4::x, mix4::b, mix4::z, mix4::d>(c2, c3);
		vector4f z_axis = vector_mix<mix4::x, mix4::b, mix4::z, mix4::d>(c4, c5);
		vector4f w_axis = vector_mix<mix4::x, mix4::b, mix4::z, mix4::d>(c6, c7);

		const scalarf det = vector_dot(x_axis, input_transposed.x_axis);
		const scalarf inv_det_s = scalar_reciprocal(det);
		const vector4f inv_det = vector_set(inv_det_s);

		x_axis = vector_mul(x_axis, inv_det);
		y_axis = vector_mul(y_axis, inv_det);
		z_axis = vector_mul(z_axis, inv_det);
		w_axis = vector_mul(w_axis, inv_det);

		return matrix4x4f{ x_axis, y_axis, z_axis, w_axis };
	}

	//////////////////////////////////////////////////////////////////////////
	// Inverses a 4x4 matrix.
	// If the input matrix has a determinant whose absolute value is below the supplied threshold, the
	// fall back value is returned instead.
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline matrix4x4f RTM_SIMD_CALL matrix_inverse(matrix4x4f_arg0 input, matrix4x4f_arg1 fallback, float threshold = 1.0E-8F) RTM_NO_EXCEPT
	{
		matrix4x4f input_transposed = matrix_transpose(input);

		vector4f v00 = vector_mix<mix4::x, mix4::x, mix4::y, mix4::y>(input_transposed.z_axis, input_transposed.z_axis);
		vector4f v01 = vector_mix<mix4::x, mix4::x, mix4::y, mix4::y>(input_transposed.x_axis, input_transposed.x_axis);
		vector4f v02 = vector_mix<mix4::x, mix4::z, mix4::a, mix4::c>(input_transposed.z_axis, input_transposed.x_axis);
		vector4f v10 = vector_mix<mix4::z, mix4::w, mix4::z, mix4::w>(input_transposed.w_axis, input_transposed.w_axis);
		vector4f v11 = vector_mix<mix4::z, mix4::w, mix4::z, mix4::w>(input_transposed.y_axis, input_transposed.y_axis);
		vector4f v12 = vector_mix<mix4::y, mix4::w, mix4::b, mix4::d>(input_transposed.w_axis, input_transposed.y_axis);

		vector4f d0 = vector_mul(v00, v10);
		vector4f d1 = vector_mul(v01, v11);
		vector4f d2 = vector_mul(v02, v12);

		v00 = vector_mix<mix4::z, mix4::w, mix4::z, mix4::w>(input_transposed.z_axis, input_transposed.z_axis);
		v01 = vector_mix<mix4::z, mix4::w, mix4::z, mix4::w>(input_transposed.x_axis, input_transposed.x_axis);
		v02 = vector_mix<mix4::y, mix4::w, mix4::b, mix4::d>(input_transposed.z_axis, input_transposed.x_axis);
		v10 = vector_mix<mix4::x, mix4::x, mix4::y, mix4::y>(input_transposed.w_axis, input_transposed.w_axis);
		v11 = vector_mix<mix4::x, mix4::x, mix4::y, mix4::y>(input_transposed.y_axis, input_transposed.y_axis);
		v12 = vector_mix<mix4::x, mix4::z, mix4::a, mix4::c>(input_transposed.w_axis, input_transposed.y_axis);

		d0 = vector_neg_mul_sub(v00, v10, d0);
		d1 = vector_neg_mul_sub(v01, v11, d1);
		d2 = vector_neg_mul_sub(v02, v12, d2);

		v00 = vector_mix<mix4::y, mix4::z, mix4::x, mix4::y>(input_transposed.y_axis, input_transposed.y_axis);
		v01 = vector_mix<mix4::z, mix4::x, mix4::y, mix4::x>(input_transposed.x_axis, input_transposed.x_axis);
		v02 = vector_mix<mix4::y, mix4::z, mix4::x, mix4::y>(input_transposed.w_axis, input_transposed.w_axis);
		vector4f v03 = vector_mix<mix4::z, mix4::x, mix4::y, mix4::x>(input_transposed.z_axis, input_transposed.z_axis);
		v10 = vector_mix<mix4::b, mix4::y, mix4::w, mix4::x>(d0, d2);
		v11 = vector_mix<mix4::w, mix4::b, mix4::y, mix4::z>(d0, d2);
		v12 = vector_mix<mix4::d, mix4::y, mix4::w, mix4::x>(d1, d2);
		vector4f v13 = vector_mix<mix4::w, mix4::d, mix4::y, mix4::z>(d1, d2);

		vector4f c0 = vector_mul(v00, v10);
		vector4f c2 = vector_mul(v01, v11);
		vector4f c4 = vector_mul(v02, v12);
		vector4f c6 = vector_mul(v03, v13);

		v00 = vector_mix<mix4::z, mix4::w, mix4::y, mix4::z>(input_transposed.y_axis, input_transposed.y_axis);
		v01 = vector_mix<mix4::w, mix4::z, mix4::w, mix4::y>(input_transposed.x_axis, input_transposed.x_axis);
		v02 = vector_mix<mix4::z, mix4::w, mix4::y, mix4::z>(input_transposed.w_axis, input_transposed.w_axis);
		v03 = vector_mix<mix4::w, mix4::z, mix4::w, mix4::y>(input_transposed.z_axis, input_transposed.z_axis);
		v10 = vector_mix<mix4::w, mix4::x, mix4::y, mix4::a>(d0, d2);
		v11 = vector_mix<mix4::z, mix4::y, mix4::a, mix4::x>(d0, d2);
		v12 = vector_mix<mix4::w, mix4::x, mix4::y, mix4::c>(d1, d2);
		v13 = vector_mix<mix4::z, mix4::y, mix4::c, mix4::x>(d1, d2);

		c0 = vector_neg_mul_sub(v00, v10, c0);
		c2 = vector_neg_mul_sub(v01, v11, c2);
		c4 = vector_neg_mul_sub(v02, v12, c4);
		c6 = vector_neg_mul_sub(v03, v13, c6);

		v00 = vector_mix<mix4::w, mix4::x, mix4::w, mix4::x>(input_transposed.y_axis, input_transposed.y_axis);
		v01 = vector_mix<mix4::y, mix4::w, mix4::x, mix4::z>(input_transposed.x_axis, input_transposed.x_axis);
		v02 = vector_mix<mix4::w, mix4::x, mix4::w, mix4::x>(input_transposed.w_axis, input_transposed.w_axis);
		v03 = vector_mix<mix4::y, mix4::w, mix4::x, mix4::z>(input_transposed.z_axis, input_transposed.z_axis);
		v10 = vector_mix<mix4::z, mix4::b, mix4::a, mix4::z>(d0, d2);
		v11 = vector_mix<mix4::b, mix4::x, mix4::w, mix4::a>(d0, d2);
		v12 = vector_mix<mix4::z, mix4::d, mix4::c, mix4::z>(d1, d2);
		v13 = vector_mix<mix4::d, mix4::x, mix4::w, mix4::c>(d1, d2);

		vector4f c1 = vector_neg_mul_sub(v00, v10, c0);
		c0 = vector_mul_add(v00, v10, c0);
		vector4f c3 = vector_mul_add(v01, v11, c2);
		c2 = vector_neg_mul_sub(v01, v11, c2);
		vector4f c5 = vector_neg_mul_sub(v02, v12, c4);
		c4 = vector_mul_add(v02, v12, c4);
		vector4f c7 = vector_mul_add(v03, v13, c6);
		c6 = vector_neg_mul_sub(v03, v13, c6);

		vector4f x_axis = vector_mix<mix4::x, mix4::b, mix4::z, mix4::d>(c0, c1);
		vector4f y_axis = vector_mix<mix4::x, mix4::b, mix4::z, mix4::d>(c2, c3);
		vector4f z_axis = vector_mix<mix4::x, mix4::b, mix4::z, mix4::d>(c4, c5);
		vector4f w_axis = vector_mix<mix4::x, mix4::b, mix4::z, mix4::d>(c6, c7);

		const scalarf det = vector_dot(x_axis, input_transposed.x_axis);
		if (scalar_cast(scalar_abs(det)) < threshold)
			return fallback;

		const scalarf inv_det_s = scalar_reciprocal(det);
		const vector4f inv_det = vector_set(inv_det_s);

		x_axis = vector_mul(x_axis, inv_det);
		y_axis = vector_mul(y_axis, inv_det);
		z_axis = vector_mul(z_axis, inv_det);
		w_axis = vector_mul(w_axis, inv_det);

		return matrix4x4f{ x_axis, y_axis, z_axis, w_axis };
	}

	//////////////////////////////////////////////////////////////////////////
	// Returns the determinant of the input 4x4 matrix.
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline scalarf RTM_SIMD_CALL matrix_determinant(matrix4x4f_arg0 input) RTM_NO_EXCEPT
	{
		matrix4x4f input_transposed = matrix_transpose(input);

		vector4f v00 = vector_mix<mix4::x, mix4::x, mix4::y, mix4::y>(input_transposed.z_axis, input_transposed.z_axis);
		vector4f v02 = vector_mix<mix4::x, mix4::z, mix4::a, mix4::c>(input_transposed.z_axis, input_transposed.x_axis);
		vector4f v10 = vector_mix<mix4::z, mix4::w, mix4::z, mix4::w>(input_transposed.w_axis, input_transposed.w_axis);
		vector4f v12 = vector_mix<mix4::y, mix4::w, mix4::b, mix4::d>(input_transposed.w_axis, input_transposed.y_axis);

		vector4f d0 = vector_mul(v00, v10);
		vector4f d2 = vector_mul(v02, v12);

		v00 = vector_mix<mix4::z, mix4::w, mix4::z, mix4::w>(input_transposed.z_axis, input_transposed.z_axis);
		v02 = vector_mix<mix4::y, mix4::w, mix4::b, mix4::d>(input_transposed.z_axis, input_transposed.x_axis);
		v10 = vector_mix<mix4::x, mix4::x, mix4::y, mix4::y>(input_transposed.w_axis, input_transposed.w_axis);
		v12 = vector_mix<mix4::x, mix4::z, mix4::a, mix4::c>(input_transposed.w_axis, input_transposed.y_axis);

		d0 = vector_neg_mul_sub(v00, v10, d0);
		d2 = vector_neg_mul_sub(v02, v12, d2);

		v00 = vector_mix<mix4::y, mix4::z, mix4::x, mix4::y>(input_transposed.y_axis, input_transposed.y_axis);
		v10 = vector_mix<mix4::b, mix4::y, mix4::w, mix4::x>(d0, d2);

		vector4f c0 = vector_mul(v00, v10);

		v00 = vector_mix<mix4::z, mix4::w, mix4::y, mix4::z>(input_transposed.y_axis, input_transposed.y_axis);
		v10 = vector_mix<mix4::w, mix4::x, mix4::y, mix4::a>(d0, d2);

		c0 = vector_neg_mul_sub(v00, v10, c0);

		v00 = vector_mix<mix4::w, mix4::x, mix4::w, mix4::x>(input_transposed.y_axis, input_transposed.y_axis);
		v10 = vector_mix<mix4::z, mix4::b, mix4::a, mix4::z>(d0, d2);

		vector4f c1 = vector_neg_mul_sub(v00, v10, c0);
		c0 = vector_mul_add(v00, v10, c0);

		vector4f x_axis = vector_mix<mix4::x, mix4::b, mix4::z, mix4::d>(c0, c1);

		return vector_dot(x_axis, input_transposed.x_axis);
	}

	//////////////////////////////////////////////////////////////////////////
	// Returns the minor of the input 4x4 matrix.
	// See: https://en.wikipedia.org/wiki/Minor_(linear_algebra)
	// The minor is the determinant of the sub-matrix input when the specified
	// row and column are removed.
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline scalarf RTM_SIMD_CALL matrix_minor(matrix4x4f_arg0 input, axis4 row, axis4 column) RTM_NO_EXCEPT
	{
		vector4f row0;
		vector4f row1;
		vector4f row2;

		// Find which rows we need.
		if (row == axis4::x)
		{
			row0 = input.y_axis;
			row1 = input.z_axis;
			row2 = input.w_axis;
		}
		else if (row == axis4::y)
		{
			row0 = input.x_axis;
			row1 = input.z_axis;
			row2 = input.w_axis;
		}
		else if (row == axis4::z)
		{
			row0 = input.x_axis;
			row1 = input.y_axis;
			row2 = input.w_axis;
		}
		else
		{
			row0 = input.x_axis;
			row1 = input.y_axis;
			row2 = input.z_axis;
		}

		// Shift our row values into a proper 3x3 matrix
		if (column == axis4::x)
		{
			row0 = vector_mix<mix4::y, mix4::z, mix4::w, mix4::w>(row0, row0);
			row1 = vector_mix<mix4::y, mix4::z, mix4::w, mix4::w>(row1, row1);
			row2 = vector_mix<mix4::y, mix4::z, mix4::w, mix4::w>(row2, row2);
		}
		else if (column == axis4::y)
		{
			row0 = vector_mix<mix4::x, mix4::z, mix4::w, mix4::w>(row0, row0);
			row1 = vector_mix<mix4::x, mix4::z, mix4::w, mix4::w>(row1, row1);
			row2 = vector_mix<mix4::x, mix4::z, mix4::w, mix4::w>(row2, row2);
		}
		else if (column == axis4::z)
		{
			row0 = vector_mix<mix4::x, mix4::y, mix4::w, mix4::w>(row0, row0);
			row1 = vector_mix<mix4::x, mix4::y, mix4::w, mix4::w>(row1, row1);
			row2 = vector_mix<mix4::x, mix4::y, mix4::w, mix4::w>(row2, row2);
		}
		else
		{
			// Already lined up
		}

		const matrix3x3f mtx = matrix_set(row0, row1, row2);
		return matrix_determinant(mtx);
	}

	//////////////////////////////////////////////////////////////////////////
	// Returns the cofactor matrix of the input 4x4 matrix.
	// See: https://en.wikipedia.org/wiki/Minor_(linear_algebra)#Cofactor_expansion_of_the_determinant
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline matrix4x4f RTM_SIMD_CALL matrix_cofactor(matrix4x4f_arg0 input) RTM_NO_EXCEPT
	{
		const scalarf minor_xx = matrix_minor(input, axis4::x, axis4::x);
		const scalarf minor_xy = matrix_minor(input, axis4::x, axis4::y);
		const scalarf minor_xz = matrix_minor(input, axis4::x, axis4::z);
		const scalarf minor_xw = matrix_minor(input, axis4::x, axis4::w);

		const scalarf minor_yx = matrix_minor(input, axis4::y, axis4::x);
		const scalarf minor_yy = matrix_minor(input, axis4::y, axis4::y);
		const scalarf minor_yz = matrix_minor(input, axis4::y, axis4::z);
		const scalarf minor_yw = matrix_minor(input, axis4::y, axis4::w);

		const scalarf minor_zx = matrix_minor(input, axis4::z, axis4::x);
		const scalarf minor_zy = matrix_minor(input, axis4::z, axis4::y);
		const scalarf minor_zz = matrix_minor(input, axis4::z, axis4::z);
		const scalarf minor_zw = matrix_minor(input, axis4::z, axis4::w);

		const scalarf minor_wx = matrix_minor(input, axis4::w, axis4::x);
		const scalarf minor_wy = matrix_minor(input, axis4::w, axis4::y);
		const scalarf minor_wz = matrix_minor(input, axis4::w, axis4::z);
		const scalarf minor_ww = matrix_minor(input, axis4::w, axis4::w);

		const vector4f xz_axis_signs = vector_set(1.0F, -1.0F, 1.0F, -1.0F);
		const vector4f yw_axis_signs = vector_set(-1.0F, 1.0F, -1.0F, 1.0F);

		const vector4f x_axis = vector_mul(vector_set(minor_xx, minor_xy, minor_xz, minor_xw), xz_axis_signs);
		const vector4f y_axis = vector_mul(vector_set(minor_yx, minor_yy, minor_yz, minor_yw), yw_axis_signs);
		const vector4f z_axis = vector_mul(vector_set(minor_zx, minor_zy, minor_zz, minor_zw), xz_axis_signs);
		const vector4f w_axis = vector_mul(vector_set(minor_wx, minor_wy, minor_wz, minor_ww), yw_axis_signs);
		return matrix4x4f{ x_axis, y_axis, z_axis, w_axis };
	}

	//////////////////////////////////////////////////////////////////////////
	// Returns the adjugate of the input matrix.
	// See: https://en.wikipedia.org/wiki/Adjugate_matrix
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline matrix4x4f RTM_SIMD_CALL matrix_adjugate(matrix4x4f_arg0 input) RTM_NO_EXCEPT
	{
		return matrix_transpose(matrix_cofactor(input));
	}

	RTM_IMPL_VERSION_NAMESPACE_END
}

RTM_IMPL_FILE_PRAGMA_POP
