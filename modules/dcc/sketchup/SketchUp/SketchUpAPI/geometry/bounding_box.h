// Copyright 2017 Trimble Inc., All rights reserved.

/**
 * @file
 * @brief Interfaces for SUBoundingBox3D.
 */
#ifndef SKETCHUP_GEOMETRY_BOUNDING_BOX_H_
#define SKETCHUP_GEOMETRY_BOUNDING_BOX_H_

#include <SketchUpAPI/geometry.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Gets the mid point from the given bounding box.
@since SketchUp 2018 M0, API 6.0
@param[in]  bounding_box The bounding box to calculate the mid point from.
@param[out] mid_point    The mid point to be returned.
@related SUBoundingBox3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if bounding_box is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if mid_point NULL
*/
SU_RESULT SUBoundingBox3DGetMidPoint(
    const struct SUBoundingBox3D* bounding_box, struct SUPoint3D* mid_point);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // SKETCHUP_GEOMETRY_BOUNDING_BOX_H_
