// Copyright 2017 Trimble Inc., All rights reserved.

/**
 * @file
 * @brief Interfaces for SURay3D.
 */
#ifndef SKETCHUP_GEOMETRY_RAY3D_H_
#define SKETCHUP_GEOMETRY_RAY3D_H_

#include <SketchUpAPI/geometry.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Gets whether or not the point is on the ray.
@since SketchUp 2018, API 6.0
@param[in]  ray   The ray.
@param[in]  point The 3D point.
@param[out] is_on Whether or not the point is on the ray.
@related SURay3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if ray or point are NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_on is NULL
*/
SU_RESULT SURay3DIsOn(const struct SURay3D* ray, const struct SUPoint3D* point, bool* is_on);

/**
@brief Gets the distance from the point to the ray.
@since SketchUp 2018, API 6.0
@param[in]  ray      The ray.
@param[in]  point    The 3D point.
@param[out] distance The distance between the ray and point.
@related SURay3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if ray or point are NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if distance is NULL
*/
SU_RESULT SURay3DDistanceTo(
    const struct SURay3D* ray, const struct SUPoint3D* point, double* distance);

/**
@brief Projects a point onto the ray.
@since SketchUp 2018, API 6.0
@param[in]  ray             The ray.
@param[in]  point           The 3D point to project onto the ray.
@param[out] projected_point The point resulting from the projection.
@related SURay3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if ray or point are NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if projected_point is NULL
*/
SU_RESULT SURay3DProjectTo(
    const struct SURay3D* ray, const struct SUPoint3D* point, struct SUPoint3D* projected_point);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // SKETCHUP_GEOMETRY_RAY3D_H_
