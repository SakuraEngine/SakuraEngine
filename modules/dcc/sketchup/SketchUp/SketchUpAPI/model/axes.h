// Copyright 2015-2020 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUAxesRef.
 */
#ifndef SKETCHUP_MODEL_AXES_H_
#define SKETCHUP_MODEL_AXES_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/transformation.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUAxesRef
@extends SUEntityRef
@brief  An axes entity reference.
@since SketchUp 2016, API 4.0
*/

/**
@brief Converts from an \ref SUAxesRef to an \ref SUEntityRef. This is
       essentially an upcast operation.
@since SketchUp 2016, API 4.0
@param[in] axes The axes object.
@related SUAxesRef
@return
- The converted \ref SUEntityRef if axes is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUAxesToEntity(SUAxesRef axes);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUAxesRef.
       This is essentially a downcast operation so the given \ref SUEntityRef
       must be convertible to an \ref SUAxesRef.
@since SketchUp 2016, API 4.0
@param[in] entity The entity object.
@related SUAxesRef
@return
- The converted \ref SUAxesRef if the downcast operation succeeds
- If the downcast operation fails, the returned reference will be invalid
*/
SU_EXPORT SUAxesRef SUAxesFromEntity(SUEntityRef entity);

/**
@brief Converts from an \ref SUAxesRef to an \ref SUDrawingElementRef.
       This is essentially an upcast operation.
@since SketchUp 2016, API 4.0
@param[in] axes The given axes reference.
@related SUAxesRef
@return
- The converted \ref SUEntityRef if axes is a valid axes
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDrawingElementRef SUAxesToDrawingElement(SUAxesRef axes);

/**
@brief Converts from an \ref SUDrawingElementRef to an \ref SUAxesRef.
       This is essentially a downcast operation so the given element must be
       convertible to an \ref SUAxesRef.
@since SketchUp 2016, API 4.0
@param[in] drawing_elem The given element reference.
@related SUAxesRef
@return
- The converted \ref SUAxesRef if the downcast operation succeeds
- If not, the returned reference will be invalid.
*/
SU_EXPORT SUAxesRef SUAxesFromDrawingElement(SUDrawingElementRef drawing_elem);

/**
@brief Creates a default constructed axes object. The axes object must be
       subsequently deallocated with \ref SUAxesRelease() unless it is
       associated with a parent object.
@since SketchUp 2016, API 4.0
@param[in] axes The axes object.
@related SUAxesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if axes is NULL
- \ref SU_ERROR_OVERWRITE_VALID if axes references a valid object
*/
SU_RESULT SUAxesCreate(SUAxesRef* axes);

/**
@brief Creates an axes object. The axes object must be
subsequently deallocated with \ref SUAxesRelease() unless it is
associated with a parent object.
@since SketchUp 2016, API 4.0
@param[in] axes The axes object.
@param[in] origin The origin of the new axes.
@param[in] xaxis The 1st axis for the custom 3D axes.
@param[in] yaxis The 2nd axis for the custom 3D axes.
@param[in] zaxis The 3rd axis for the custom 3D axes.
@related SUAxesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if axes is NULL
- \ref SU_ERROR_OVERWRITE_VALID if axes references a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if any of the input pointers (origin, xaxis
  yaxis, and zaxis) are NULL
- \ref SU_ERROR_GENERIC if the three vectors don't make an orthogonal axes
*/
SU_RESULT SUAxesCreateCustom(
    SUAxesRef* axes, const struct SUPoint3D* origin, const struct SUVector3D* xaxis,
    const struct SUVector3D* yaxis, const struct SUVector3D* zaxis);

/**
@brief Releases aa axes object. The axes object must have been created with
       \ref SUAxesCreate() and not subsequently associated with a parent object
       (e.g. \ref SUEntitiesRef).
@since SketchUp 2016, API 4.0
@param[in] axes The axes object.
@related SUAxesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if axes is NULL
- \ref SU_ERROR_INVALID_INPUT if axes does not reference a valid object
*/
SU_RESULT SUAxesRelease(SUAxesRef* axes);

/**
@brief  Retrieves the origin point value, not a reference.
@since SketchUp 2016, API 4.0
@param[in] axes The axes object.
@param[out] origin Pointer to a \ref SUPoint3D struct for returning the origin.
@related SUAxesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if axes is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if origin is NULL
*/
SU_RESULT SUAxesGetOrigin(SUAxesRef axes, struct SUPoint3D* origin);

/**
@brief  Sets the origin point value for the provided axes.
@since SketchUp 2016, API 4.0
@param[in] axes The axes object.
@param[in] origin Pointer to a \ref SUPoint3D struct for setting the origin.
@related SUAxesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if axes is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if origin is NULL
*/
SU_RESULT SUAxesSetOrigin(SUAxesRef axes, const struct SUPoint3D* origin);

/**
@brief  Retrieves the 1st axis vector value, not a reference.
@since SketchUp 2016, API 4.0
@param[in] axes The axes object.
@param[out] axis Pointer to a \ref SUVector3D struct for getting the 1st axis.
@related SUAxesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if axes is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if axis is NULL
*/
SU_RESULT SUAxesGetXAxis(SUAxesRef axes, struct SUVector3D* axis);

/**
@brief  Retrieves the 2nd axis vector value, not a reference.
@since SketchUp 2016, API 4.0
@param[in] axes The axes object.
@param[out] axis Pointer to a \ref SUVector3D struct for getting the 2nd axis.
@related SUAxesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if axes is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if axis is NULL
*/
SU_RESULT SUAxesGetYAxis(SUAxesRef axes, struct SUVector3D* axis);

/**
@brief  Retrieves the 3rd axis vector value, not a reference.
@since SketchUp 2016, API 4.0
@param[in] axes The axes object.
@param[out] axis Pointer to a \ref SUVector3D struct for getting the 3rd axis.
@related SUAxesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if axes is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if axis is NULL
*/
SU_RESULT SUAxesGetZAxis(SUAxesRef axes, struct SUVector3D* axis);

/**
@brief  Sets the axes' vectors. Fails if vectors don't make an orthogonal axes.
@since SketchUp 2016, API 4.0
@param[in] axes The axes object.
@param[in] xaxis Pointer to a \ref SUVector3D struct for setting the 1st axis.
@param[in] yaxis Pointer to a \ref SUVector3D struct for setting the 2nd axis.
@param[in] zaxis Pointer to a \ref SUVector3D struct for setting the 3rd axis.
@related SUAxesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if axes is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if xaxis, yaxis, or zaxis is NULL
- \ref SU_ERROR_GENERIC if the three vectors don't make an orthogonal axes
*/
SU_RESULT SUAxesSetAxesVecs(
    SUAxesRef axes, const struct SUVector3D* xaxis, const struct SUVector3D* yaxis,
    const struct SUVector3D* zaxis);

/**
@brief  Retrieves a copy of the transformation.
@since SketchUp 2016, API 4.0
@param[in] axes The axes object.
@param[out] transform Pointer to a \ref SUTransformation struct for getting the
            transformation data.
@related SUAxesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if axes is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
*/
SU_RESULT SUAxesGetTransform(SUAxesRef axes, struct SUTransformation* transform);

/**
@brief  Retrieves a copy of the plane.
@since SketchUp 2016, API 4.0
@param[in] axes The axes object.
@param[out] plane Pointer to a \ref SUPlane3D struct for getting the
plane data.
@related SUAxesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if axes is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if plane is NULL
*/
SU_RESULT SUAxesGetPlane(SUAxesRef axes, struct SUPlane3D* plane);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_AXES_H_
