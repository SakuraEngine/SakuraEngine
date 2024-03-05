// Copyright 2017 Trimble Inc., All rights reserved.

/**
 * @file
 * @brief Interfaces for SUPlane3D.
 */
#ifndef SKETCHUP_GEOMETRY_PLANE3D_H_
#define SKETCHUP_GEOMETRY_PLANE3D_H_

#include <SketchUpAPI/geometry.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Sets the plane using three points.
@since SketchUp 2018, API 6.0
@param[out] plane  The plane defined by the three points.
@param[in]  point1 The first point.
@param[in]  point2 The second point.
@param[in]  point3 The third point.
@related SUPlane3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if plane is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if point1, point2, or point3 are NULL
*/
SU_RESULT SUPlane3DSetWithPoints(
    struct SUPlane3D* plane, const struct SUPoint3D* point1, const struct SUPoint3D* point2,
    const struct SUPoint3D* point3);

/**
@brief Sets the plane using a point and normal vector.
@since SketchUp 2018, API 6.0
@param[out] plane  The plane defined by the point and normal.
@param[in]  point  The point.
@param[in]  normal The normal vector.
@related SUPlane3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if plane is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if point or normal are NULL
*/
SU_RESULT SUPlane3DSetWithPointAndNormal(
    struct SUPlane3D* plane, const struct SUPoint3D* point, const struct SUVector3D* normal);

/**
@brief Sets the plane using equation coefficients.
@since SketchUp 2018, API 6.0
@param[out] plane  The plane defined by the coefficients.
@param[in]  a      The first coefficient.
@param[in]  b      The second coefficient.
@param[in]  c      The third coefficient.
@param[in]  d      The fourth coefficient.
@related SUPlane3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if plane is NULL
*/
SU_RESULT SUPlane3DSetWithCoefficients(
    struct SUPlane3D* plane, double a, double b, double c, double d);

/**
@brief Gets the position on the plane closest to the origin.
@since SketchUp 2016, API 4.0
@param[in]  plane    The plane from which to get the position.
@param[out] position The 3D point struct retrieved.
@related SUPlane3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if plane is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if position is NULL
*/
SU_RESULT SUPlane3DGetPosition(const struct SUPlane3D* plane, struct SUPoint3D* position);

/**
@brief Gets the plane's unit normal vector.
@since SketchUp 2017, API 5.0
@param[in]  plane  The plane from which to get the normal.
@param[out] normal The 3D vector struct retrieved.
@related SUPlane3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if plane is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if normal is NULL
*/
SU_RESULT SUPlane3DGetNormal(const struct SUPlane3D* plane, struct SUVector3D* normal);

/**
@brief Gets whether or not the point is on the plane.
@since SketchUp 2018, API 6.0
@param[in]  plane The plane.
@param[in]  point The 3D point.
@param[out] is_on Whether or not the point is on the plane.
@related SUPlane3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if plane or point are NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_on is NULL
*/
SU_RESULT SUPlane3DIsOn(const struct SUPlane3D* plane, const struct SUPoint3D* point, bool* is_on);

/**
@brief Gets the distance from the point to the nearest point on the plane.
@since SketchUp 2018, API 6.0
@param[in]  plane    The plane.
@param[in]  point    The 3D point.
@param[out] distance The distance between the plane and point.
@related SUPlane3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if plane or point are NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if distance is NULL
*/
SU_RESULT SUPlane3DDistanceTo(
    const struct SUPlane3D* plane, const struct SUPoint3D* point, double* distance);

/**
@brief Projects a point onto the plane.
@since SketchUp 2018, API 6.0
@param[in]  plane           The plane.
@param[in]  point           The 3D point to project onto the plane.
@param[out] projected_point The point resulting from the projection.
@related SUPlane3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if plane or point are NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if projected_point is NULL
*/
SU_RESULT SUPlane3DProjectTo(
    const struct SUPlane3D* plane, const struct SUPoint3D* point,
    struct SUPoint3D* projected_point);

/**
@brief Transforms the provided 3D plane by applying the provided 3D
       transformation.
@since SketchUp 2018, API 6.0
@param[in]     transform The transformation to be applied.
@param[in,out] plane     The plane to be transformed.
@related SUPlane3D
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if transform is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if plane is NULL
*/
SU_RESULT SUPlane3DTransform(const struct SUTransformation* transform, struct SUPlane3D* plane);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // SKETCHUP_GEOMETRY_PLANE3D_H_
