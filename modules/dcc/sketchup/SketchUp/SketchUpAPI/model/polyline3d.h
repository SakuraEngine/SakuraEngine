// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUPolyline3dRef.
 */
#ifndef SKETCHUP_MODEL_POLYLINE3D_H_
#define SKETCHUP_MODEL_POLYLINE3D_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUPolyline3dRef
@extends SUEntityRef
@brief  A polyline3d object. These are curve-like entities that do not
        generate inference snaps or affect geometry in any way.
*/

/**
@brief  Converts from an \ref SUPolyline3dRef to an \ref SUEntityRef. This is
        essentially an upcast operation.
@param[in] line The polyline3d object.
@related SUPolyline3dRef
@return
- The converted \ref SUEntityRef if line is a valid object. If not, the returned
  reference will be invalid.
*/
SU_EXPORT SUEntityRef SUPolyline3dToEntity(SUPolyline3dRef line);

/**
@brief  Converts from an \ref SUEntityRef to an \ref SUPolyline3dRef. This is
        essentially a downcast operation so the given entity must be convertible
        to an \ref SUPolyline3dRef.
@param[in] entity The given entity reference.
@related SUPolyline3dRef
@return
- The converted \ref SUPolyline3dRef if the downcast operation succeeds. If not,
  the returned reference will be invalid.
*/
SU_EXPORT SUPolyline3dRef SUPolyline3dFromEntity(SUEntityRef entity);

/**
@brief  Converts from an \ref SUPolyline3dRef to an \ref SUDrawingElementRef.
        This is essentially an upcast operation.
@param[in] line The polyline3d object.
@related SUPolyline3dRef
@return
- The converted \ref SUEntityRef if line is a valid object. If not, the returned
  reference will be invalid.
*/
SU_EXPORT SUDrawingElementRef SUPolyline3dToDrawingElement(SUPolyline3dRef line);

/**
@brief  Converts from an \ref SUDrawingElementRef to an \ref SUPolyline3dRef.
        This is essentially a downcast operation so the given element must be
        convertible to an \ref SUPolyline3dRef.
@param[in] drawing_elem The drawing element object.
@related SUPolyline3dRef
@return
- The converted \ref SUPolyline3dRef if the downcast operation succeeds. If not,
  the returned reference will be invalid.
*/
SU_EXPORT SUPolyline3dRef SUPolyline3dFromDrawingElement(SUDrawingElementRef drawing_elem);

/**
@brief Creates a new polyline3d object. The polyline3d object must be
       subsequently deallocated with \ref SUPolyline3dRelease() unless it is
       associated with a parent object.
@param[out] polyline The polyline3d object.
@related SUPolyline3dRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if polyline is NULL
*/
SU_RESULT SUPolyline3dCreate(SUPolyline3dRef* polyline);

/**
@brief Releases a new polyline3d object. The polyline3d object must not be
       associated with a parent object.
@param[in] polyline The polyline3d object.
@related SUPolyline3dRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if polyline does not reference a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if polyline is NULL
*/
SU_RESULT SUPolyline3dRelease(SUPolyline3dRef* polyline);

/**
@brief Adds points to a polyline3d object.
@param[in] polyline   The polyline3d object.
@param[in] num_points Number of points being added.
@param[in] points     Array of points to add.
@related SUPolyline3dRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if polyline is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if points is NULL
*/
SU_RESULT SUPolyline3dAddPoints(
    SUPolyline3dRef polyline, size_t num_points, struct SUPoint3D points[]);

/**
@brief  Retrieves the number of points contained by a polyline3d.
@param[in]  line  The polyline3d object.
@param[out] count The number of points available.
@related SUPolyline3dRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if line is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUPolyline3dGetNumPoints(SUPolyline3dRef line, size_t* count);

/**
@brief  Retrieves the points in the polyline3d object.
@param[in]  line   The polyline3d object.
@param[in]  len    The maximum number of points to retrieve.
@param[out] points The points retrieved.
@param[out] count  The number of points retrieved.
@related SUPolyline3dRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if line is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if points or count is NULL
*/
SU_RESULT SUPolyline3dGetPoints(
    SUPolyline3dRef line, size_t len, struct SUPoint3D points[], size_t* count);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_POLYLINE3D_H_
