// Copyright 2015-2020 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUArcCurveRef.
 */
#ifndef SKETCHUP_MODEL_ARCCURVE_H_
#define SKETCHUP_MODEL_ARCCURVE_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>

// Forward declarations
struct SUPoint3D;
struct SUVector3D;

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUArcCurveRef
@extends SUCurveRef
@brief  References an arccurve.
*/

/**
@brief Converts from an \ref SUArcCurveRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@since SketchUp 2016, API 4.0
@param[in] arccurve The given arccurve reference.
@related SUArcCurveRef
@return
- The converted \ref SUEntityRef if curve is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUArcCurveToEntity(SUArcCurveRef arccurve);

/**
@brief Converts from an \ref SUEntityRef to an SUArcCurveRef.
       This is essentially a downcast operation so the given \ref SUEntityRef
       must be convertible to an \ref SUArcCurveRef.
@since SketchUp 2016, API 4.0
@param[in] entity The given entity reference.
@related SUArcCurveRef
@return
- The converted \ref SUArcCurveRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUArcCurveRef SUArcCurveFromEntity(SUEntityRef entity);

/**
@brief Converts from an \ref SUArcCurveRef to an SUCurveRef.
       This is essentially an upcast operation.
@since SketchUp 2016, API 4.0
@param[in] arccurve The given arccurve reference.
@related SUArcCurveRef
@return
- The converted SUCurveRef if arccurve is a valid arccurve object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUCurveRef SUArcCurveToCurve(SUArcCurveRef arccurve);

/**
@brief Converts from an SUCurveRef to an \ref SUArcCurveRef.
       This is essentially a downcast operation so the given SUCurveRef
       must be convertible to an \ref SUArcCurveRef.
@since SketchUp 2016, API 4.0
@param[in] curve The given curve reference.
@related SUArcCurveRef
@return
- The converted \ref SUArcCurveRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUArcCurveRef SUArcCurveFromCurve(SUCurveRef curve);

/**
@brief Creates an arccurve object. If the start and end points are the same a
       full circle will be generated.
@since SketchUp 2016, API 4.0
@param[out] arccurve    The arccurve object created.
@param[in]  center      The point at the center of the arc.
@param[in]  start_point The point at the start of the arc.
@param[in]  end_point   The point at the end of the arc.
@param[in]  normal      The vector normal to the arc plane.
@param[in]  num_edges   The number of edges for the arc.
@related SUArcCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if arccurve is NULL
- \ref SU_ERROR_OVERWRITE_VALID if arccurve already references a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if center, start_point, end_point, or normal
                                   are NULL
*/
SU_RESULT SUArcCurveCreate(
    SUArcCurveRef* arccurve, const struct SUPoint3D* center, const struct SUPoint3D* start_point,
    const struct SUPoint3D* end_point, const struct SUVector3D* normal, size_t num_edges);

/**
@brief  Releases an arccurve object and its associated edge objects.
@since SketchUp 2016, API 4.0
@param[in,out] arccurve The arccurve object.
@related SUArcCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if arccurve is NULL
- \ref SU_ERROR_INVALID_INPUT if arccurve does not reference a valid object
*/
SU_RESULT SUArcCurveRelease(SUArcCurveRef* arccurve);

/**
@brief  Retrieves the raduis.
@since SketchUp 2016, API 4.0
@param[in]  arccurve The arccurve object.
@param[out] radius   The arccurve radius.
@related SUArcCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if arccurve is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if radius is NULL
*/
SU_RESULT SUArcCurveGetRadius(SUArcCurveRef arccurve, double* radius);

/**
@brief  Retrieves the starting point.
@since SketchUp 2016, API 4.0
@param[in]  arccurve The arccurve object.
@param[out] point    The arccurve starting point.
@related SUArcCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if arccurve is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
SU_RESULT SUArcCurveGetStartPoint(SUArcCurveRef arccurve, struct SUPoint3D* point);

/**
@brief  Retrieves the ending point.
@since SketchUp 2016, API 4.0
@param[in]  arccurve The arccurve object.
@param[out] point    The arccurve ending point.
@related SUArcCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if arccurve is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
SU_RESULT SUArcCurveGetEndPoint(SUArcCurveRef arccurve, struct SUPoint3D* point);

/**
@brief  Retrieves the x-axis.
@since SketchUp 2016, API 4.0
@param[in]  arccurve The arccurve object.
@param[out] axis     The arccurve x-axis.
@related SUArcCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if arccurve is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if axis is NULL
*/
SU_RESULT SUArcCurveGetXAxis(SUArcCurveRef arccurve, struct SUVector3D* axis);

/**
@brief  Retrieves the y-axis.
@since SketchUp 2016, API 4.0
@param[in]  arccurve The arccurve object.
@param[out] axis     The arccurve y-axis.
@related SUArcCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if arccurve is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if axis is NULL
*/
SU_RESULT SUArcCurveGetYAxis(SUArcCurveRef arccurve, struct SUVector3D* axis);

/**
@brief  Retrieves the center point.
@since SketchUp 2016, API 4.0
@param[in]  arccurve The arccurve object.
@param[out] point    The arccurve center point.
@related SUArcCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if arccurve is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
SU_RESULT SUArcCurveGetCenter(SUArcCurveRef arccurve, struct SUPoint3D* point);

/**
@brief  Retrieves the normal.
@since SketchUp 2016, API 4.0
@param[in]  arccurve The arccurve object.
@param[out] normal   The arccurve normal vector.
@related SUArcCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if arccurve is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if normal is NULL
*/
SU_RESULT SUArcCurveGetNormal(SUArcCurveRef arccurve, struct SUVector3D* normal);

/**
@brief  Retrieves the start angle.
@since SketchUp 2016, API 4.0
@param[in]  arccurve The arccurve object.
@param[out] angle    The arccurve start angle.
@related SUArcCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if arccurve is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if angle is NULL
*/
SU_RESULT SUArcCurveGetStartAngle(SUArcCurveRef arccurve, double* angle);

/**
@brief  Retrieves the end angle.
@since SketchUp 2016, API 4.0
@param[in]  arccurve The arccurve object.
@param[out] angle    The arccurve end angle.
@related SUArcCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if arccurve is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if angle is NULL
*/
SU_RESULT SUArcCurveGetEndAngle(SUArcCurveRef arccurve, double* angle);

/**a boolean indicating if the arccurve is a full circle.
@since SketchUp 2016, API 4.0
@param[in]  arccurve The arccurve object.
@param[out] is_full  Returns true if the arccurve is a full corcle.
@related SUArcCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if arccurve is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_full is NULL
*/
SU_RESULT SUArcCurveGetIsFullCircle(SUArcCurveRef arccurve, bool* is_full);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_ARCCURVE_H_
