// Copyright 2016 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUDimensionRadialRef.
 */
#ifndef SKETCHUP_MODEL_DIMENSION_RADIAL_H_
#define SKETCHUP_MODEL_DIMENSION_RADIAL_H_

#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUDimensionRadialRef
@extends SUDimensionRef
@brief  A radial dimension entity reference.
@since SketchUp 2017, API 5.0
*/

/**
@brief Converts from an \ref SUDimensionRadialRef to an \ref SUDimensionRef.
       This is essentially an upcast operation.
@since SketchUp 2017, API 5.0
@param[in] dimension The given dimension reference.
@related SUDimensionRadialRef
@return
- The converted \ref SUDimensionRef if dimension is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDimensionRef SUDimensionRadialToDimension(SUDimensionRadialRef dimension);

/**
@brief Converts from an SUDimensionRef to an \ref SUDimensionRadialRef. This is
       essentially a downcast operation so the given SUDimensionRef must be
       convertible to an \ref SUDimensionRadialRef.
@since SketchUp 2017, API 5.0
@param[in] dimension The given dimension reference.
@related SUDimensionRadialRef
@return
- The converted \ref SUDimensionRadialRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDimensionRadialRef SUDimensionRadialFromDimension(SUDimensionRef dimension);

/**
@brief Creates a new radial dimension for measuring the provided arccurve.
@since SketchUp 2017, API 5.0
@param[in,out] dimension The dimension object created.
@param[in]  path      The and instance path to the arccurve to be measured.
@related SUDimensionRadialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if dimension is NULL
- \ref SU_ERROR_OVERWRITE_VALID if dimension already references a valid object
- \ref SU_ERROR_INVALID_INPUT if path is not a valid object
- \ref SU_ERROR_INVALID_ARGUMENT if path is valid but refers to an invalid
       instance path
- \ref SU_ERROR_GENERIC if path refers to a valid instance path but the path's
       leaf is not an arccurve
*/
SU_RESULT SUDimensionRadialCreate(SUDimensionRadialRef* dimension, SUInstancePathRef path);

/**
@brief Releases a dimension object.
@since SketchUp 2017, API 5.0
@param[in] dimension The dimension object.
@related SUDimensionRadialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if dimension is NULL
*/
SU_RESULT SUDimensionRadialRelease(SUDimensionRadialRef* dimension);

/**
@brief Retrieves the arccurve instance being mesured by a dimension object. The
       given instance path object either must have been constructed using one
       of the SUInstancePathCreate* functions or it will be generated on the
       fly if it is invalid. It must be released using
       SUInstancePathRelease() when it is no longer needed.
@since SketchUp 2017, API 5.0
@param[in]  dimension The dimension object.
@param[out] path      The instance path retrieved.
@related SUDimensionRadialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if path is NULL
*/
SU_RESULT SUDimensionRadialGetCurveInstancePath(
    SUDimensionRadialRef dimension, SUInstancePathRef* path);

/**
@brief Sets which arccurve instance is measured by the radial dimension. The
       instance path's leaf entity must be an arccurve for this method to
       succeed.
@since SketchUp 2017, API 5.0
@param[in] dimension The dimension object modified.
@param[in] path      The and instance path to the arccurve to be measured.
@related SUDimensionRadialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension or path are not valid objects
- \ref SU_ERROR_INVALID_ARGUMENT if path is valid but refers to an invalid
       instance path
- \ref SU_ERROR_GENERIC if path refers to a valid instance path but the path's
       leaf is not an arccurve
*/
SU_RESULT SUDimensionRadialSetCurveInstancePath(
    SUDimensionRadialRef dimension, SUInstancePathRef path);

/**
@brief Retrieves whether the dimension is a diameter.  Radial dimensions can be
       used to measure either diameter or radius.
@since SketchUp 2017, API 5.0
@param[in]  dimension   The dimension object.
@param[out] is_diameter The flag value retrieved.
@related SUDimensionRadialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_diameter is NULL
*/
SU_RESULT SUDimensionRadialGetDiameter(SUDimensionRadialRef dimension, bool* is_diameter);

/**
@brief Sets whether the dimension measures diameter or radius.
@since SketchUp 2017, API 5.0
@param[in] dimension   The dimension object.
@param[in] is_diameter The flag specifying if the dimension measures diameter.
@related SUDimensionRadialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
*/
SU_RESULT SUDimensionRadialSetDiameter(SUDimensionRadialRef dimension, bool is_diameter);

/**
@brief Gets the radial dimension's leader line break point. The leader line
       break point is the point where the leader line bends towards the
       dimension label.
@since SketchUp 2017, API 5.0
@param[in]  dimension The dimension object.
@param[out] point     The point retrieved.
@related SUDimensionRadialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
SU_RESULT SUDimensionRadialGetLeaderBreakPoint(
    SUDimensionRadialRef dimension, struct SUPoint3D* point);

/**
@brief Sets the radial dimension's leader break point
@since SketchUp 2017, API 5.0
@param[in] dimension The dimension object.
@param[in] point     The point retrieved.
@related SUDimensionRadialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if point is NULL
*/
SU_RESULT SUDimensionRadialSetLeaderBreakPoint(
    SUDimensionRadialRef dimension, const struct SUPoint3D* point);

/**
@brief Retrieves the a dimension object's leader points.  The three returned
       pointe are [0] the point at which the dimension's text touches the
       leader line, [1] the point at which the dimension's arrow attaches to
       the dimensioned curve, [2] the point on the dimensioned curve's full
       circle opposite of point [1].
@since SketchUp 2017, API 5.0
@param[in]  dimension The dimension object.
@param[out] points    The array of 3 3d points retrieved.
@related SUDimensionRadialRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if points is NULL
*/
SU_RESULT SUDimensionRadialGetLeaderPoints(
    SUDimensionRadialRef dimension, struct SUPoint3D points[3]);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_DIMENSION_RADIAL_H_
