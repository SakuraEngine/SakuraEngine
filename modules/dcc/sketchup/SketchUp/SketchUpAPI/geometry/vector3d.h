// Copyright 2017 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUVector3D.
 */
#ifndef SKETCHUP_GEOMETRY_VECTOR3D_H_
#define SKETCHUP_GEOMETRY_VECTOR3D_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Creates a vector between two point objects.
@since SketchUp 2018, API 6.0
@param[in]  from   The first point object.
@param[in]  to     The second point object.
@param[out] vector The vector from from to to.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if from or to is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vector is NULL
*/
SU_RESULT SUVector3DCreate(
    struct SUVector3D* vector, const struct SUPoint3D* from, const struct SUPoint3D* to);

/**
@brief Determines if a vector is valid. A vector is invalid if its length is
       zero.
@since SketchUp 2018, API 6.0
@param[in]  vector The vector object.
@param[out] valid  Whether the vector is valid.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if valid is NULL
*/
SU_RESULT SUVector3DIsValid(const struct SUVector3D* vector, bool* valid);

/**
@brief Determines if two vectors are parallel.
@since SketchUp 2018, API 6.0
@param[in]  vector1  The first vector object.
@param[in]  vector2  The second vector object.
@param[out] parallel Whether the vectors are parallel.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if parallel is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if vector1 or vector2 is not a valid vector
*/
SU_RESULT SUVector3DIsParallelTo(
    const struct SUVector3D* vector1, const struct SUVector3D* vector2, bool* parallel);

/**
@brief Determines if two vectors are perpendicular.
@since SketchUp 2018, API 6.0
@param[in]  vector1       The first vector object.
@param[in]  vector2       The second vector object.
@param[out] perpendicular Whether the vectors are perpendicular.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if perpendicular is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if vector1 or vector2 is not a valid vector
*/
SU_RESULT SUVector3DIsPerpendicularTo(
    const struct SUVector3D* vector1, const struct SUVector3D* vector2, bool* perpendicular);

/**
@brief Determines if two vectors are parallel and pointing the same direction.
@since SketchUp 2018, API 6.0
@param[in]  vector1        The first vector object.
@param[in]  vector2        The second vector object.
@param[out] same_direction Whether the vectors are pointing in the same direction.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if same_direction is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if vector1 or vector2 is not a valid vector
*/
SU_RESULT SUVector3DIsSameDirectionAs(
    const struct SUVector3D* vector1, const struct SUVector3D* vector2, bool* same_direction);

/**
@brief Determines if two vectors are equal.
@since SketchUp 2018, API 6.0
@param[in]  vector1 The first vector object.
@param[in]  vector2 The second vector object.
@param[out] equal   Whether the vectors are equal.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if equal is NULL
*/
SU_RESULT SUVector3DIsEqual(
    const struct SUVector3D* vector1, const struct SUVector3D* vector2, bool* equal);

/**
@brief Determines if vector1 is less than vector2.
@param[in]  vector1   The first vector object.
@param[in]  vector2   The second vector object.
@param[out] less_than Whether vector1 is less than vector2.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if less_than is NULL
*/
SU_RESULT SUVector3DLessThan(
    const struct SUVector3D* vector1, const struct SUVector3D* vector2, bool* less_than);

/**
@brief Normalizes a vector.
@since SketchUp 2018, API 6.0
@param[in,out] vector The vector object.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if vector is not a valid vector
*/
SU_RESULT SUVector3DNormalize(struct SUVector3D* vector);

/**
@brief Reverses a vector.
@since SketchUp 2018, API 6.0
@param[in,out] vector The vector object.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
*/
SU_RESULT SUVector3DReverse(struct SUVector3D* vector);

/**
@brief Computes the dot product of two vectors.
@since SketchUp 2018, API 6.0
@param[in]  vector1 The first vector object.
@param[in]  vector2 The second vector object.
@param[out] dot     The value of the dot product.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if dot is NULL
*/
SU_RESULT SUVector3DDot(
    const struct SUVector3D* vector1, const struct SUVector3D* vector2, double* dot);

/**
@brief Computes the cross product of two vectors.
@since SketchUp 2018, API 6.0
@param[in]  vector1 The first vector object.
@param[in]  vector2 The second vector object.
@param[out] cross   The value of the cross product.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if cross is NULL
*/
SU_RESULT SUVector3DCross(
    const struct SUVector3D* vector1, const struct SUVector3D* vector2, struct SUVector3D* cross);

/**
@brief Determines if a vector has a length of one.
@since SketchUp 2018, API 6.0
@param[in]  vector         The vector object.
@param[out] is_unit_vector Whether the vector has a length of one.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_unit_vector is NULL
*/
SU_RESULT SUVector3DIsUnitVector(const struct SUVector3D* vector, bool* is_unit_vector);

/**
@brief Gets the length of a vector.
@since SketchUp 2018, API 6.0
@param[in]  vector The vector object.
@param[out] length The length of the vector.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if length is NULL
*/
SU_RESULT SUVector3DGetLength(const struct SUVector3D* vector, double* length);

/**
@brief Sets the length of a vector.
@since SketchUp 2018, API 6.0
@param[in,out] vector The vector object.
@param[in]     length The new length the vector should be.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if vector is not a valid vector
*/
SU_RESULT SUVector3DSetLength(struct SUVector3D* vector, double length);

/**
@brief Gets the angle between two vectors.
@since SketchUp 2018, API 6.0
@param[in]  vector1 The first vector object.
@param[in]  vector2 The second vector object.
@param[out] angle   The angle between the vectors.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if angle is NULL
*/
SU_RESULT SUVector3DAngleBetween(
    const struct SUVector3D* vector1, const struct SUVector3D* vector2, double* angle);

/**
@brief Get arbitrary axes perpendicular to this vector. This method uses the
       arbitrary axis algorithm to calculate the x and y vectors.
@param[in]  z_axis The vector to use as the z axis.
@param[out] x_axis The computed x axis vector.
@param[out] y_axis The computed y axis vector.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if z_axis is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if x_axis or y_axis is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if z_axis is not a valid vector
*/
SU_RESULT SUVector3DGetArbitraryAxes(
    const struct SUVector3D* z_axis, struct SUVector3D* x_axis, struct SUVector3D* y_axis);

/**
@brief Transforms the provided 3D vector by applying the provided 3D
       transformation.
@since SketchUp 2018, API 6.0
@param[in]     transform The transformation to be applied.
@param[in,out] vector    The vector to be transformed.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vector is NULL
*/
SU_RESULT SUVector3DTransform(const struct SUTransformation* transform, struct SUVector3D* vector);

/**
@brief Creates a new vector as a linear combination of other vectors.
       This method is generally used to get a vector at some percentage between
       two vectors.
@since SketchUp 2018, API 6.0
@param[in]  vectors An array of vectors. Must be size of 2 or 3.
@param[in]  weights An array of weights that correspond to the percentage. Must
                    be the same size as \p vectors.
@param[in]  size    The size of the \p vectors and \p weights array.
@param[out] vector  The new computed vector.
@related SUVector3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_OUT_OF_RANGE if size is less than 2 or greater than 3
- \ref SU_ERROR_NULL_POINTER_INPUT if vectors or weights is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vector is NULL
*/
SU_RESULT SUVector3DLinearCombination(
    const struct SUVector3D* vectors, const double* weights, const size_t size,
    struct SUVector3D* vector);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_GEOMETRY_VECTOR3D_H_
