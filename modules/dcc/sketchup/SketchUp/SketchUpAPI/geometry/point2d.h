// Copyright 2017 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUPoint2D.
 */
#ifndef SKETCHUP_GEOMETRY_POINT2D_H_
#define SKETCHUP_GEOMETRY_POINT2D_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Creates a vector between two point objects.
@since SketchUp 2017, API 5.0
@deprecated The functionality is replaced by SUVector2DCreate.
@param[in]  point1   The first point object.
@param[in]  point2   The second point object.
@param[out] vector   The vector from point1 to point2.
@related SUPoint2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if point1 or point2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vector is NULL
 */
SU_DEPRECATED_FUNCTION("SketchUp API 6.0")
SU_RESULT SUPoint2DToSUPoint2D(
    const struct SUPoint2D* point1, const struct SUPoint2D* point2, struct SUVector2D* vector);

/**
@brief Determines if two points are equal.
@since SketchUp 2017, API 5.0
@param[in]  point1 The first point object.
@param[in]  point2 The second point object.
@param[out] equal  Whether the two points are the same.
@related SUPoint2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if point1 or point2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if equal is NULL
*/
SU_RESULT SUPoint2DGetEqual(
    const struct SUPoint2D* point1, const struct SUPoint2D* point2, bool* equal);

/**
@brief Creates a new point that is offset from another point.
@since SketchUp 2017, API 5.0
@param[in]  point1 The point object.
@param[in]  vector The offset vector object.
@param[out] point2 The new point.
@related SUPoint2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if point1 or vector is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point2 is NULL
 */
SU_RESULT SUPoint2DOffset(
    const struct SUPoint2D* point1, const struct SUVector2D* vector, struct SUPoint2D* point2);

/**
@brief Gets the distance between two point objects.
@since SketchUp 2017, API 5.0
@param[in]  point1   The first point object.
@param[in]  point2   The second point object.
@param[out] distance The distance between the two points.
@related SUPoint2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if point1 or point2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if distance is NULL
 */
SU_RESULT SUPoint2DDistanceToSUPoint2D(
    const struct SUPoint2D* point1, const struct SUPoint2D* point2, double* distance);

/**
@brief Transforms a point by applying a 2D transformation.
@since SketchUp 2019, API 7.0
@param[in]     transform The transformation to be applied.
@param[in,out] point     The point to be transformed.
@related SUPoint2D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
SU_RESULT SUPoint2DTransform(const struct SUTransformation2D* transform, struct SUPoint2D* point);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_GEOMETRY_POINT2D_H_
