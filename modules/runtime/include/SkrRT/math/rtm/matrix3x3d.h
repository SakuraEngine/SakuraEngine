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

#include "SkrRT/math/rtm/math.h"
#include "SkrRT/math/rtm/vector4d.h"
#include "SkrRT/math/rtm/version.h"
#include "SkrRT/math/rtm/impl/compiler_utils.h"
#include "SkrRT/math/rtm/impl/matrix_common.h"
#include "SkrRT/math/rtm/impl/matrix_affine_common.h"

RTM_IMPL_FILE_PRAGMA_PUSH

namespace rtm
{
	RTM_IMPL_VERSION_NAMESPACE_BEGIN

	//////////////////////////////////////////////////////////////////////////
	// Returns the desired 3x3 matrix axis.
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK RTM_FORCE_INLINE constexpr const vector4d& matrix_get_axis(const matrix3x3d& input, axis3 axis) RTM_NO_EXCEPT
	{
		return axis == axis3::x ? input.x_axis : (axis == axis3::y ? input.y_axis : input.z_axis);
	}

	//////////////////////////////////////////////////////////////////////////
	// Converts a 3x3 matrix into a rotation quaternion.
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline quatd quat_from_matrix(const matrix3x3d& input) RTM_NO_EXCEPT
	{
		return rtm_impl::quat_from_matrix(input.x_axis, input.y_axis, input.z_axis);
	}

	//////////////////////////////////////////////////////////////////////////
	// Multiplies two 3x3 matrices.
	// Multiplication order is as follow: local_to_world = matrix_mul(local_to_object, object_to_world)
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline matrix3x3d RTM_SIMD_CALL matrix_mul(const matrix3x3d& lhs, const matrix3x3d& rhs) RTM_NO_EXCEPT
	{
		vector4d tmp = vector_mul(vector_dup_x(lhs.x_axis), rhs.x_axis);
		tmp = vector_mul_add(vector_dup_y(lhs.x_axis), rhs.y_axis, tmp);
		tmp = vector_mul_add(vector_dup_z(lhs.x_axis), rhs.z_axis, tmp);
		vector4d x_axis = tmp;

		tmp = vector_mul(vector_dup_x(lhs.y_axis), rhs.x_axis);
		tmp = vector_mul_add(vector_dup_y(lhs.y_axis), rhs.y_axis, tmp);
		tmp = vector_mul_add(vector_dup_z(lhs.y_axis), rhs.z_axis, tmp);
		vector4d y_axis = tmp;

		tmp = vector_mul(vector_dup_x(lhs.z_axis), rhs.x_axis);
		tmp = vector_mul_add(vector_dup_y(lhs.z_axis), rhs.y_axis, tmp);
		tmp = vector_mul_add(vector_dup_z(lhs.z_axis), rhs.z_axis, tmp);
		vector4d z_axis = tmp;

		return matrix3x3d{ x_axis, y_axis, z_axis };
	}

	//////////////////////////////////////////////////////////////////////////
	// Multiplies a 3x3 matrix and a 3D vector.
	// Multiplication order is as follow: world_position = matrix_mul(local_vector, local_to_world)
	// Note: The proper way to transform a normal by a matrix with non-uniform scale
	// is to multiply the normal with the cofactor matrix.
	// See: https://github.com/graphitemaster/normals_revisited
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK RTM_FORCE_INLINE vector4d RTM_SIMD_CALL matrix_mul_vector3(const vector4d& vec3, const matrix3x3d& mtx) RTM_NO_EXCEPT
	{
		vector4d tmp;

		tmp = vector_mul(vector_dup_x(vec3), mtx.x_axis);
		tmp = vector_mul_add(vector_dup_y(vec3), mtx.y_axis, tmp);
		tmp = vector_mul_add(vector_dup_z(vec3), mtx.z_axis, tmp);

		return tmp;
	}

	//////////////////////////////////////////////////////////////////////////
	// Transposes a 3x3 matrix.
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK RTM_FORCE_INLINE matrix3x3d RTM_SIMD_CALL matrix_transpose(const matrix3x3d& input) RTM_NO_EXCEPT
	{
		const vector4d v00_v01_v10_v11 = vector_mix<mix4::x, mix4::y, mix4::a, mix4::b>(input.x_axis, input.y_axis);
		const vector4d v02_v03_v12_v13 = vector_mix<mix4::z, mix4::w, mix4::c, mix4::d>(input.x_axis, input.y_axis);

		const vector4d x_axis = vector_mix<mix4::x, mix4::z, mix4::a, mix4::c>(v00_v01_v10_v11, input.z_axis);
		const vector4d y_axis = vector_mix<mix4::y, mix4::w, mix4::b, mix4::d>(v00_v01_v10_v11, input.z_axis);
		const vector4d z_axis = vector_mix<mix4::x, mix4::z, mix4::c, mix4::c>(v02_v03_v12_v13, input.z_axis);
		return matrix3x3d{ x_axis, y_axis, z_axis };
	}

	//////////////////////////////////////////////////////////////////////////
	// Inverses a 3x3 matrix.
	// If the input matrix is not invertible, the result is undefined.
	// For a safe alternative, supply a fallback value and a threshold.
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline matrix3x3d RTM_SIMD_CALL matrix_inverse(const matrix3x3d& input) RTM_NO_EXCEPT
	{
		const vector4d v00_v01_v10_v11 = vector_mix<mix4::x, mix4::y, mix4::a, mix4::b>(input.x_axis, input.y_axis);
		const vector4d v02_v03_v12_v13 = vector_mix<mix4::z, mix4::w, mix4::c, mix4::d>(input.x_axis, input.y_axis);

		const vector4d v00_v10_v20_v22 = vector_mix<mix4::x, mix4::z, mix4::a, mix4::c>(v00_v01_v10_v11, input.z_axis);
		const vector4d v01_v11_v21_v23 = vector_mix<mix4::y, mix4::w, mix4::b, mix4::d>(v00_v01_v10_v11, input.z_axis);
		const vector4d v02_v12_v22_v22 = vector_mix<mix4::x, mix4::z, mix4::c, mix4::c>(v02_v03_v12_v13, input.z_axis);

		const vector4d v11_v21_v01 = vector_mix<mix4::y, mix4::z, mix4::b, mix4::c>(v01_v11_v21_v23, input.x_axis);
		const vector4d v22_v02_v12_v10 = vector_mix<mix4::z, mix4::x, mix4::c, mix4::a>(v02_v12_v22_v22, input.y_axis);
		const vector4d v11v22_v21v02_v01v12 = vector_mul(v11_v21_v01, v22_v02_v12_v10);

		const vector4d v01_v02_v11_v12 = vector_mix<mix4::y, mix4::z, mix4::b, mix4::c>(input.x_axis, input.y_axis);

		const vector4d v12_v01_v11 = vector_mix<mix4::w, mix4::x, mix4::b, mix4::c>(v01_v02_v11_v12, v01_v11_v21_v23);
		const vector4d v21_v22_v02 = vector_mix<mix4::y, mix4::z, mix4::b, mix4::c>(input.z_axis, v22_v02_v12_v10);

		vector4d x_axis = vector_neg_mul_sub(v12_v01_v11, v21_v22_v02, v11v22_v21v02_v01v12);

		const vector4d v20_v00_v10_v22 = vector_mix<mix4::z, mix4::x, mix4::d, mix4::a>(v00_v10_v20_v22, v22_v02_v12_v10);
		const vector4d v12_v22_v02 = vector_mix<mix4::y, mix4::z, mix4::c, mix4::c>(v02_v12_v22_v22, v21_v22_v02);
		const vector4d v20v12_v00v22_v10v02 = vector_mul(v20_v00_v10_v22, v12_v22_v02);

		const vector4d v10_v02_v00 = vector_mix<mix4::w, mix4::y, mix4::b, mix4::c>(v22_v02_v12_v10, v20_v00_v10_v22);
		const vector4d v22_v20_v12 = vector_mix<mix4::w, mix4::x, mix4::a, mix4::c>(v20_v00_v10_v22, v12_v22_v02);

		vector4d y_axis = vector_neg_mul_sub(v10_v02_v00, v22_v20_v12, v20v12_v00v22_v10v02);

		const vector4d v10_v20_v00 = vector_mix<mix4::z, mix4::x, mix4::c, mix4::c>(v20_v00_v10_v22, v10_v02_v00);
		const vector4d v21_v01_v11 = vector_mix<mix4::y, mix4::z, mix4::c, mix4::c>(v11_v21_v01, v12_v01_v11);
		const vector4d v10v21_v20v01_v00v11 = vector_mul(v10_v20_v00, v21_v01_v11);

		const vector4d v20_v00_v01 = vector_mix<mix4::y, mix4::z, mix4::c, mix4::c>(v10_v20_v00, v11_v21_v01);
		const vector4d v11_v21_v10 = vector_mix<mix4::x, mix4::y, mix4::a, mix4::c>(v11_v21_v01, v10_v20_v00);

		vector4d z_axis = vector_neg_mul_sub(v20_v00_v01, v11_v21_v10, v10v21_v20v01_v00v11);

		const vector4d o00_o00_o10_o10 = vector_mix<mix4::x, mix4::x, mix4::a, mix4::a>(x_axis, y_axis);
		const vector4d o00_o10_o20 = vector_mix<mix4::x, mix4::z, mix4::a, mix4::a>(o00_o00_o10_o10, z_axis);

		const double det = vector_dot3(o00_o10_o20, input.x_axis);
		const vector4d inv_det = vector_set(scalar_reciprocal(det));

		x_axis = vector_mul(x_axis, inv_det);
		y_axis = vector_mul(y_axis, inv_det);
		z_axis = vector_mul(z_axis, inv_det);

		return matrix3x3d{ x_axis, y_axis, z_axis };
	}

	//////////////////////////////////////////////////////////////////////////
	// Inverses a 3x3 matrix.
	// If the input matrix has a determinant whose absolute value is below the supplied threshold, the
	// fall back value is returned instead.
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline matrix3x3d RTM_SIMD_CALL matrix_inverse(const matrix3x3d& input, const matrix3x3d& fallback, double threshold = 1.0E-8) RTM_NO_EXCEPT
	{
		const vector4d v00_v01_v10_v11 = vector_mix<mix4::x, mix4::y, mix4::a, mix4::b>(input.x_axis, input.y_axis);
		const vector4d v02_v03_v12_v13 = vector_mix<mix4::z, mix4::w, mix4::c, mix4::d>(input.x_axis, input.y_axis);

		const vector4d v00_v10_v20_v22 = vector_mix<mix4::x, mix4::z, mix4::a, mix4::c>(v00_v01_v10_v11, input.z_axis);
		const vector4d v01_v11_v21_v23 = vector_mix<mix4::y, mix4::w, mix4::b, mix4::d>(v00_v01_v10_v11, input.z_axis);
		const vector4d v02_v12_v22_v22 = vector_mix<mix4::x, mix4::z, mix4::c, mix4::c>(v02_v03_v12_v13, input.z_axis);

		const vector4d v11_v21_v01 = vector_mix<mix4::y, mix4::z, mix4::b, mix4::c>(v01_v11_v21_v23, input.x_axis);
		const vector4d v22_v02_v12_v10 = vector_mix<mix4::z, mix4::x, mix4::c, mix4::a>(v02_v12_v22_v22, input.y_axis);
		const vector4d v11v22_v21v02_v01v12 = vector_mul(v11_v21_v01, v22_v02_v12_v10);

		const vector4d v01_v02_v11_v12 = vector_mix<mix4::y, mix4::z, mix4::b, mix4::c>(input.x_axis, input.y_axis);

		const vector4d v12_v01_v11 = vector_mix<mix4::w, mix4::x, mix4::b, mix4::c>(v01_v02_v11_v12, v01_v11_v21_v23);
		const vector4d v21_v22_v02 = vector_mix<mix4::y, mix4::z, mix4::b, mix4::c>(input.z_axis, v22_v02_v12_v10);

		vector4d x_axis = vector_neg_mul_sub(v12_v01_v11, v21_v22_v02, v11v22_v21v02_v01v12);

		const vector4d v20_v00_v10_v22 = vector_mix<mix4::z, mix4::x, mix4::d, mix4::a>(v00_v10_v20_v22, v22_v02_v12_v10);
		const vector4d v12_v22_v02 = vector_mix<mix4::y, mix4::z, mix4::c, mix4::c>(v02_v12_v22_v22, v21_v22_v02);
		const vector4d v20v12_v00v22_v10v02 = vector_mul(v20_v00_v10_v22, v12_v22_v02);

		const vector4d v10_v02_v00 = vector_mix<mix4::w, mix4::y, mix4::b, mix4::c>(v22_v02_v12_v10, v20_v00_v10_v22);
		const vector4d v22_v20_v12 = vector_mix<mix4::w, mix4::x, mix4::a, mix4::c>(v20_v00_v10_v22, v12_v22_v02);

		vector4d y_axis = vector_neg_mul_sub(v10_v02_v00, v22_v20_v12, v20v12_v00v22_v10v02);

		const vector4d v10_v20_v00 = vector_mix<mix4::z, mix4::x, mix4::c, mix4::c>(v20_v00_v10_v22, v10_v02_v00);
		const vector4d v21_v01_v11 = vector_mix<mix4::y, mix4::z, mix4::c, mix4::c>(v11_v21_v01, v12_v01_v11);
		const vector4d v10v21_v20v01_v00v11 = vector_mul(v10_v20_v00, v21_v01_v11);

		const vector4d v20_v00_v01 = vector_mix<mix4::y, mix4::z, mix4::c, mix4::c>(v10_v20_v00, v11_v21_v01);
		const vector4d v11_v21_v10 = vector_mix<mix4::x, mix4::y, mix4::a, mix4::c>(v11_v21_v01, v10_v20_v00);

		vector4d z_axis = vector_neg_mul_sub(v20_v00_v01, v11_v21_v10, v10v21_v20v01_v00v11);

		const vector4d o00_o00_o10_o10 = vector_mix<mix4::x, mix4::x, mix4::a, mix4::a>(x_axis, y_axis);
		const vector4d o00_o10_o20 = vector_mix<mix4::x, mix4::z, mix4::a, mix4::a>(o00_o00_o10_o10, z_axis);

		const double det = vector_dot3(o00_o10_o20, input.x_axis);
		if (scalar_abs(det) < threshold)
			return fallback;

		const vector4d inv_det = vector_set(scalar_reciprocal(det));

		x_axis = vector_mul(x_axis, inv_det);
		y_axis = vector_mul(y_axis, inv_det);
		z_axis = vector_mul(z_axis, inv_det);

		return matrix3x3d{ x_axis, y_axis, z_axis };
	}

	//////////////////////////////////////////////////////////////////////////
	// Returns the determinant of the input 3x3 matrix.
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline scalard matrix_determinant(const matrix3x3d& input) RTM_NO_EXCEPT
	{
		const vector4d v00_v01_v10_v11 = vector_mix<mix4::x, mix4::y, mix4::a, mix4::b>(input.x_axis, input.y_axis);
		const vector4d v02_v03_v12_v13 = vector_mix<mix4::z, mix4::w, mix4::c, mix4::d>(input.x_axis, input.y_axis);

		const vector4d v00_v10_v20_v22 = vector_mix<mix4::x, mix4::z, mix4::a, mix4::c>(v00_v01_v10_v11, input.z_axis);
		const vector4d v01_v11_v21_v23 = vector_mix<mix4::y, mix4::w, mix4::b, mix4::d>(v00_v01_v10_v11, input.z_axis);
		const vector4d v02_v12_v22_v22 = vector_mix<mix4::x, mix4::z, mix4::c, mix4::c>(v02_v03_v12_v13, input.z_axis);

		const vector4d v11_v21_v01 = vector_mix<mix4::y, mix4::z, mix4::b, mix4::c>(v01_v11_v21_v23, input.x_axis);
		const vector4d v22_v02_v12_v10 = vector_mix<mix4::z, mix4::x, mix4::c, mix4::a>(v02_v12_v22_v22, input.y_axis);
		const vector4d v11v22_v21v02_v01v12 = vector_mul(v11_v21_v01, v22_v02_v12_v10);

		const vector4d v01_v02_v11_v12 = vector_mix<mix4::y, mix4::z, mix4::b, mix4::c>(input.x_axis, input.y_axis);

		const vector4d v12_v01_v11 = vector_mix<mix4::w, mix4::x, mix4::b, mix4::c>(v01_v02_v11_v12, v01_v11_v21_v23);
		const vector4d v21_v22_v02 = vector_mix<mix4::y, mix4::z, mix4::b, mix4::c>(input.z_axis, v22_v02_v12_v10);

		vector4d x_axis = vector_neg_mul_sub(v12_v01_v11, v21_v22_v02, v11v22_v21v02_v01v12);

		const vector4d v20_v00_v10_v22 = vector_mix<mix4::z, mix4::x, mix4::d, mix4::a>(v00_v10_v20_v22, v22_v02_v12_v10);
		const vector4d v12_v22_v02 = vector_mix<mix4::y, mix4::z, mix4::c, mix4::c>(v02_v12_v22_v22, v21_v22_v02);
		const vector4d v20v12_v00v22_v10v02 = vector_mul(v20_v00_v10_v22, v12_v22_v02);

		const vector4d v10_v02_v00 = vector_mix<mix4::w, mix4::y, mix4::b, mix4::c>(v22_v02_v12_v10, v20_v00_v10_v22);
		const vector4d v22_v20_v12 = vector_mix<mix4::w, mix4::x, mix4::a, mix4::c>(v20_v00_v10_v22, v12_v22_v02);

		vector4d y_axis = vector_neg_mul_sub(v10_v02_v00, v22_v20_v12, v20v12_v00v22_v10v02);

		const vector4d v10_v20_v00 = vector_mix<mix4::z, mix4::x, mix4::c, mix4::c>(v20_v00_v10_v22, v10_v02_v00);
		const vector4d v21_v01_v11 = vector_mix<mix4::y, mix4::z, mix4::c, mix4::c>(v11_v21_v01, v12_v01_v11);
		const vector4d v10v21_v20v01_v00v11 = vector_mul(v10_v20_v00, v21_v01_v11);

		const vector4d v20_v00_v01 = vector_mix<mix4::y, mix4::z, mix4::c, mix4::c>(v10_v20_v00, v11_v21_v01);
		const vector4d v11_v21_v10 = vector_mix<mix4::x, mix4::y, mix4::a, mix4::c>(v11_v21_v01, v10_v20_v00);

		vector4d z_axis = vector_neg_mul_sub(v20_v00_v01, v11_v21_v10, v10v21_v20v01_v00v11);

		const vector4d o00_o00_o10_o10 = vector_mix<mix4::x, mix4::x, mix4::a, mix4::a>(x_axis, y_axis);
		const vector4d o00_o10_o20 = vector_mix<mix4::x, mix4::z, mix4::a, mix4::a>(o00_o00_o10_o10, z_axis);

		return vector_dot3(o00_o10_o20, input.x_axis);
	}

	//////////////////////////////////////////////////////////////////////////
	// Returns the minor of the input 3x3 matrix.
	// See: https://en.wikipedia.org/wiki/Minor_(linear_algebra)
	// The minor is the determinant of the sub-matrix input when the specified
	// row and column are removed.
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline scalard matrix_minor(const matrix3x3d& input, axis3 row, axis3 column) RTM_NO_EXCEPT
	{
		// The minor boils down to calculating the determinant of a 2x2 matrix.
		// det([a, b], [c, d]) = (a * d) - (b * c)

		// Find which two rows we need.
		vector4d row0;
		vector4d row1;
		if (row == axis3::x)
		{
			row0 = input.y_axis;
			row1 = input.z_axis;
		}
		else if (row == axis3::y)
		{
			row0 = input.x_axis;
			row1 = input.z_axis;
		}
		else
		{
			row0 = input.x_axis;
			row1 = input.y_axis;
		}

		// Because our input is a 3x3 matrix, there are only a few possibilities for the 2x2 part:
		// row0 = [x0, y0, z0]
		// row1 = [x1, y1, z1]
		// det([x0, y0], [x1, y1]) = (x0 * y1) - (y0 * x1) (z removed)
		// det([x0, z0], [x1, z1]) = (x0 * z1) - (z0 * x1) (y removed)
		// det([y0, z0], [y1, z1]) = (y0 * z1) - (z0 * y1) (x removed)
		// det([column0, column1], [column2, column3]) = (column0 * column3) - (column1 * column2)

		// For performance reasons, we can compute all three determinants at the same time.
		const vector4d column0 = vector_mix<mix4::x, mix4::x, mix4::y, mix4::y>(row0, row0);
		const vector4d column1 = vector_mix<mix4::y, mix4::z, mix4::z, mix4::z>(row0, row0);
		const vector4d column2 = vector_mix<mix4::x, mix4::x, mix4::y, mix4::y>(row1, row1);
		const vector4d column3 = vector_mix<mix4::y, mix4::z, mix4::z, mix4::z>(row1, row1);

		const vector4d determinants = vector_neg_mul_sub(column1, column2, vector_mul(column0, column3));

		// Extract the one we need
		if (column == axis3::x)
			return vector_get_z(determinants);
		else if (column == axis3::y)
			return vector_get_y(determinants);
		else
			return vector_get_x(determinants);
	}

	//////////////////////////////////////////////////////////////////////////
	// Returns the cofactor matrix of the input 3x3 matrix.
	// See: https://en.wikipedia.org/wiki/Minor_(linear_algebra)#Cofactor_expansion_of_the_determinant
	// Note: The proper way to transform a normal by a matrix with non-uniform scale
	// is to multiply the normal with the cofactor matrix.
	// See: https://github.com/graphitemaster/normals_revisited
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline matrix3x3d matrix_cofactor(const matrix3x3d& input) RTM_NO_EXCEPT
	{
		const vector4d x_axis = vector_cross3(input.y_axis, input.z_axis);
		const vector4d y_axis = vector_cross3(input.z_axis, input.x_axis);
		const vector4d z_axis = vector_cross3(input.x_axis, input.y_axis);
		return matrix3x3d{ x_axis, y_axis, z_axis };
	}

	//////////////////////////////////////////////////////////////////////////
	// Returns the adjugate of the input matrix.
	// See: https://en.wikipedia.org/wiki/Adjugate_matrix
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline matrix3x3d matrix_adjugate(const matrix3x3d& input) RTM_NO_EXCEPT
	{
		return matrix_transpose(matrix_cofactor(input));
	}

	//////////////////////////////////////////////////////////////////////////
	// Removes the 3D scale from a 3x3 matrix.
	// Note that if the scaling is 0.0 for a particular axis, the original rotation axis cannot
	// be recovered trivially and no attempt is done to do so. In theory, we could use the other axes
	// to try and recover it.
	// TODO: Implement rotation recovering, perhaps in a separate function and rename this
	// one to matrix_remove_non_zero_scale(..)
	//////////////////////////////////////////////////////////////////////////
	RTM_DISABLE_SECURITY_COOKIE_CHECK inline matrix3x3d matrix_remove_scale(const matrix3x3d& input) RTM_NO_EXCEPT
	{
		matrix3x3d result;
		result.x_axis = vector_normalize3(input.x_axis, input.x_axis);
		result.y_axis = vector_normalize3(input.y_axis, input.y_axis);
		result.z_axis = vector_normalize3(input.z_axis, input.z_axis);
		return result;
	}

	RTM_IMPL_VERSION_NAMESPACE_END
}

RTM_IMPL_FILE_PRAGMA_POP
