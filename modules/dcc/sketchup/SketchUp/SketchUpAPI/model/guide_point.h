// Copyright 2014 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUGuidePointRef.
 */
#ifndef SKETCHUP_MODEL_GUIDE_POINT_H_
#define SKETCHUP_MODEL_GUIDE_POINT_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUGuidePointRef
@extends SUDrawingElementRef
@brief  A guide point that has a position.
@since SketchUp 2014 M1, API 2.1
*/

/**
@brief Converts from an \ref SUGuidePointRef to an \ref SUEntityRef. This is
       essentially an upcast operation.
@since SketchUp 2014 M1, API 2.1
@param[in] guide_point The guide point object.
@related SUGuidePointRef
@return
- The converted \ref SUEntityRef if guide_point is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUGuidePointToEntity(SUGuidePointRef guide_point);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUGuidePointRef.
       This is essentially a downcast operation so the given \ref SUEntityRef
       must be convertible to an \ref SUGuidePointRef.
@since SketchUp 2014 M1, API 2.1
@param[in] entity The entity object.
@related SUGuidePointRef
@return
- The converted \ref SUGuidePointRef if the downcast operation succeeds
- If the downcast operation fails, the returned reference will be invalid
*/
SU_EXPORT SUGuidePointRef SUGuidePointFromEntity(SUEntityRef entity);

/**
@brief Converts from an \ref SUGuidePointRef to an \ref SUDrawingElementRef.
       This is essentially an upcast operation.
@since SketchUp 2015 M0, API 3.0
@param[in] guide_point The given guide point reference.
@related SUGuidePointRef
@return
- The converted \ref SUEntityRef if guide_point is a valid guide point.
- If not, the returned reference will be invalid.
*/
SU_EXPORT SUDrawingElementRef SUGuidePointToDrawingElement(SUGuidePointRef guide_point);

/**
@brief Converts from an \ref SUDrawingElementRef to an \ref SUGuidePointRef.
       This is essentially a downcast operation so the given element must be
       convertible to an SUGuidePointRef.
@since SketchUp 2015 M0, API 3.0
@param[in] drawing_elem The given element reference.
@related SUGuidePointRef
@return
- The converted \ref SUGuidePointRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUGuidePointRef SUGuidePointFromDrawingElement(SUDrawingElementRef drawing_elem);

/**
@brief Creates a guide point object. The guide point object must be subsequently
       deallocated with \ref SUGuidePointRelease() unless it is associated with
       a parent object.
@since SketchUp 2014 M1, API 2.1
@param[in]  guide_point The guide point object.
@param[out] position    The guide point position.
@related SUGuidePointRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if guide_point is NULL
- \ref SU_ERROR_OVERWRITE_VALID if guide_point references a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if position is NULL
*/
SU_RESULT SUGuidePointCreate(SUGuidePointRef* guide_point, const struct SUPoint3D* position);

/**
@brief Releases a guide point object. The guide point object must have been
       created with SUGuidePointCreate() and not subsequently associated
       with a parent object (e.g. SUEntitiesAddGuidePoints()).
@since SketchUp 2014 M1, API 2.1
@param[in] guide_point The guide point object.
@related SUGuidePointRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if guide_point is NULL
- \ref SU_ERROR_INVALID_INPUT if guide_point does not reference a valid object
*/
SU_RESULT SUGuidePointRelease(SUGuidePointRef* guide_point);

/**
@brief Retrieves the position of a guide point object.
@since SketchUp 2014 M1, API 2.1
@param[in]  guide_point The guide point object.
@param[out] position    The guide point position.
@related SUGuidePointRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if guide point is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if position is NULL
*/
SU_RESULT SUGuidePointGetPosition(SUGuidePointRef guide_point, struct SUPoint3D* position);

/**
@brief Retrieves the anchor position of a guide point object. If the point was
       created in SketchUp then the anchor is the position that was first
       clicked during the point creation. If the point was created with \ref
       SUGuidePointCreate the anchor is the origin.
@since SketchUp 2016, API 4.0
@param[in]  guide_point The guide point object.
@param[out] position    The guide point anchor position.
@related SUGuidePointRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if guide point is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if position is NULL
*/
SU_RESULT SUGuidePointGetFromPosition(SUGuidePointRef guide_point, struct SUPoint3D* position);

/**
@brief Retrieves the boolean indicating if the point should be displayed as a
       line.
@since SketchUp 2016, API 4.0
@param[in]  guide_point The guide point object.
@param[out] as_line     Return true if the point is set to be displayed as a
                        line.
@related SUGuidePointRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if guide point is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if as_line is NULL
*/
SU_RESULT SUGuidePointGetDisplayAsLine(SUGuidePointRef guide_point, bool* as_line);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_GUIDE_POINT_H_
