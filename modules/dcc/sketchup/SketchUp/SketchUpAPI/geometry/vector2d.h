// Copyright 2017 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUVector2D.
 */
#ifndef SKETCHUP_GEOMETRY_VECTOR2D_H_
#define SKETCHUP_GEOMETRY_VECTOR2D_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Creates a vector between two point objects.
@since SketchUp 2018 M0, API 6.0
@param[in]  from   The first point object.
@param[in]  to     The second point object.
@param[out] vector The vector from from to to.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if from or to is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vector is NULL
*/
SU_RESULT SUVector2DCreate(
    struct SUVector2D* vector, const struct SUPoint2D* from, const struct SUPoint2D* to);

/**
@brief Determines if a vector is valid. A vector is invalid if its length is
       zero.
@since SketchUp 2017, API 5.0
@param[in]  vector The vector object.
@param[out] valid  Whether the vector is valid.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if valid is NULL
*/
SU_RESULT SUVector2DIsValid(const struct SUVector2D* vector, bool* valid);

/**
@brief Determines if two vectors are parallel.
@since SketchUp 2017, API 5.0
@param[in]  vector1  The first vector object.
@param[in]  vector2  The second vector object.
@param[out] parallel Whether the vectors are parallel.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if parallel is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if vector1 or vector2 is not a valid vector
*/
SU_RESULT SUVector2DIsParallelTo(
    const struct SUVector2D* vector1, const struct SUVector2D* vector2, bool* parallel);

/**
@brief Determines if two vectors are perpendicular.
@since SketchUp 2017, API 5.0
@param[in]  vector1       The first vector object.
@param[in]  vector2       The second vector object.
@param[out] perpendicular Whether the vectors are perpendicular.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if perpendicular is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if vector1 or vector2 is not a valid vector
*/
SU_RESULT SUVector2DIsPerpendicularTo(
    const struct SUVector2D* vector1, const struct SUVector2D* vector2, bool* perpendicular);

/**
@brief Determines if two vectors are parallel and pointing the same direction.
@since SketchUp 2017, API 5.0
@param[in]  vector1        The first vector object.
@param[in]  vector2        The second vector object.
@param[out] same_direction Whether the vectors are pointing in the same direction.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if same_direction is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if vector1 or vector2 is not a valid vector
*/
SU_RESULT SUVector2DIsSameDirectionAs(
    const struct SUVector2D* vector1, const struct SUVector2D* vector2, bool* same_direction);

/**
@brief Determines if two vectors are equal.
@since SketchUp 2018, API 6.0
@param[in]  vector1 The first vector object.
@param[in]  vector2 The second vector object.
@param[out] equal   Whether the vectors are equal.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if equal is NULL
*/
SU_RESULT SUVector2DIsEqual(
    const struct SUVector2D* vector1, const struct SUVector2D* vector2, bool* equal);

/**
@brief Normalizes a vector.
@since SketchUp 2017, API 5.0
@param[in,out] vector The vector object.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if vector is not a valid vector
*/
SU_RESULT SUVector2DNormalize(struct SUVector2D* vector);

/**
@brief Reverses a vector.
@since SketchUp 2017, API 5.0
@param[in] vector The vector object.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
*/
SU_RESULT SUVector2DReverse(struct SUVector2D* vector);

/**
@brief Computes the dot product of two vectors.
@since SketchUp 2017, API 5.0
@param[in]  vector1 The first vector object.
@param[in]  vector2 The second vector object.
@param[out] dot     The value of the dot product.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if dot is NULL
*/
SU_RESULT SUVector2DDot(
    const struct SUVector2D* vector1, const struct SUVector2D* vector2, double* dot);

/**
@brief Computes the cross product of two vectors.
@since SketchUp 2017, API 5.0
@param[in]  vector1 The first vector object.
@param[in]  vector2 The second vector object.
@param[out] cross   The value of the cross product.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if cross is NULL
*/
SU_RESULT SUVector2DCross(
    const struct SUVector2D* vector1, const struct SUVector2D* vector2, double* cross);

/**
@brief Determines if a vector has a length of one.
@since SketchUp 2017, API 5.0
@param[in]  vector         The vector object.
@param[out] is_unit_vector Whether the vector has a length of one.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_unit_vector is NULL
*/
SU_RESULT SUVector2DIsUnitVector(const struct SUVector2D* vector, bool* is_unit_vector);

/**
@brief Gets the length of a vector.
@since SketchUp 2017, API 5.0
@param[in]  vector The vector object.
@param[out] length The length of the vector.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if length is NULL
*/
SU_RESULT SUVector2DGetLength(const struct SUVector2D* vector, double* length);

/**
@brief Sets the length of a vector.
@since SketchUp 2017, API 5.0
@param[in,out] vector The vector object.
@param[in]     length The new length the vector should be.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
- \ref SU_ERROR_OUT_OF_RANGE if length is zero
- \ref SU_ERROR_INVALID_ARGUMENT if vector is not a valid vector
*/
SU_RESULT SUVector2DSetLength(struct SUVector2D* vector, double length);

/**
@brief Gets the angle between two vectors.
@since SketchUp 2017, API 5.0
@param[in]  vector1 The first vector object.
@param[in]  vector2 The second vector object.
@param[out] angle   The angle between the vectors.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vector1 or vector2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if angle is NULL
*/
SU_RESULT SUVector2DAngleBetween(
    const struct SUVector2D* vector1, const struct SUVector2D* vector2, double* angle);

/**
@brief Transforms a vector by applying a 2D transformation.
@since SketchUp 2019, API 7.0
@param[in]     transform The transformation to be applied.
@param[in,out] vector    The vector to be transformed.
@related SUVector2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vector is NULL
*/
SU_RESULT SUVector2DTransform(
    const struct SUTransformation2D* transform, struct SUVector2D* vector);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_GEOMETRY_VECTOR2D_H_
