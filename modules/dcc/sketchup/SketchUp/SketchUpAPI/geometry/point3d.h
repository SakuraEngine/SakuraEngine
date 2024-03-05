// Copyright 2017 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUPoint3D.
 */
#ifndef SKETCHUP_GEOMETRY_POINT3D_H_
#define SKETCHUP_GEOMETRY_POINT3D_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>

// Forward declarations
struct SUTransformation;

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Determines if two points are equal.
@since SketchUp 2018 M0, API 6.0
@param[in]  point1 The first point object.
@param[in]  point2 The second point object.
@param[out] equal  Whether the two points are the same.
@related SUPoint3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if point1 or point2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if equal is NULL
*/
SU_RESULT SUPoint3DGetEqual(
    const struct SUPoint3D* point1, const struct SUPoint3D* point2, bool* equal);

/**
@brief Determines if point1 is less than point2.
@since SketchUp 2018 M0, API 6.0
@param[in]  point1    The first point object.
@param[in]  point2    The second point object.
@param[out] less_than Whether point1 is less than point2.
@related SUPoint3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if point1 or point2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if less_than is NULL
*/
SU_RESULT SUPoint3DLessThan(
    const struct SUPoint3D* point1, const struct SUPoint3D* point2, bool* less_than);

/**
@brief Creates a new point that is offset from another point.
@since SketchUp 2018 M0, API 6.0
@param[in]  point1 The point object.
@param[in]  vector The offset vector object.
@param[out] point2 The new point.
@related SUPoint3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if point1 or vector is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point2 is NULL
 */
SU_RESULT SUPoint3DOffset(
    const struct SUPoint3D* point1, const struct SUVector3D* vector, struct SUPoint3D* point2);

/**
@brief Gets the distance between two point objects.
@since SketchUp 2018 M0, API 6.0
@param[in]  point1   The first point object.
@param[in]  point2   The second point object.
@param[out] distance The distance between the two points.
@related SUPoint3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if point1 or point2 is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if distance is NULL
 */
SU_RESULT SUPoint3DDistanceToSUPoint3D(
    const struct SUPoint3D* point1, const struct SUPoint3D* point2, double* distance);

/**
@brief Transforms the provided 3D point by applying the provided 3D
       transformation.
@since SketchUp 2016, API 4.0
@param[in]     transform The transformation to be applied.
@param[in,out] point     The point to be transformed.
@related SUPoint3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
SU_RESULT SUPoint3DTransform(const struct SUTransformation* transform, struct SUPoint3D* point);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_GEOMETRY_POINT3D_H_
