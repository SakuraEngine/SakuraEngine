// Copyright 2014 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUGuideLineRef.
 */
#ifndef SKETCHUP_MODEL_GUIDE_LINE_H_
#define SKETCHUP_MODEL_GUIDE_LINE_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUGuideLineRef
@extends SUDrawingElementRef
@brief  A guide line that has a start and end position.
@since SketchUp 2016, API 4.0
*/

/**
@brief Converts from an \ref SUGuideLineRef to an \ref SUEntityRef. This is
       essentially an upcast operation.
@since SketchUp 2016, API 4.0
@param[in] guide_line The guide line object.
@related SUGuideLineRef
@return
- The converted \ref SUEntityRef if guide_line is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUGuideLineToEntity(SUGuideLineRef guide_line);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUGuideLineRef.
       This is essentially a downcast operation so the given \ref SUEntityRef
       must be convertible to an \ref SUGuideLineRef.
@since SketchUp 2016, API 4.0
@param[in] entity The entity object.
@related SUGuideLineRef
@return
- The converted \ref SUGuideLineRef if the downcast operation succeeds
- If the downcast operation fails, the returned reference will be invalid
*/
SU_EXPORT SUGuideLineRef SUGuideLineFromEntity(SUEntityRef entity);

/**
@brief Converts from an \ref SUGuideLineRef to an \ref SUDrawingElementRef.
       This is essentially an upcast operation.
@since SketchUp 2016, API 4.0
@param[in] guide_line The given guide line reference.
@related SUGuideLineRef
@return
- The converted \ref SUEntityRef if guide_line is a valid guide line
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDrawingElementRef SUGuideLineToDrawingElement(SUGuideLineRef guide_line);

/**
@brief Converts from an \ref SUDrawingElementRef to an \ref SUGuideLineRef.
       This is essentially a downcast operation so the given element must be
       convertible to an \ref SUGuideLineRef.
@since SketchUp 2016, API 4.0
@param[in] drawing_elem The given element reference.
@related SUGuideLineRef
@return
- The converted \ref SUGuideLineRef if the downcast operation succeeds
- If not, the returned reference will be invalid.
*/
SU_EXPORT SUGuideLineRef SUGuideLineFromDrawingElement(SUDrawingElementRef drawing_elem);

/**
@brief Creates a finite guide line object. The guide line object must be
       subsequently deallocated with \ref SUGuideLineRelease() unless it is
       associated with a parent object.  The generated line will be a segment
       with start and end points.  The end point can be obtained by adding the
       direction vector to the start point.
@since SketchUp 2016, API 4.0
@param[in]  guide_line The guide line object.
@param[out] start      The guide line start position.
@param[out] end        The guide line end position.
@related SUGuideLineRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if guide_line is NULL
- \ref SU_ERROR_OVERWRITE_VALID if guide_line references a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if start or end is NULL
*/
SU_RESULT SUGuideLineCreateFinite(
    SUGuideLineRef* guide_line, const struct SUPoint3D* start, const struct SUPoint3D* end);

/**
@brief Creates a infinite guide line object. The guide line object must be
       subsequently deallocated with \ref SUGuideLineRelease() unless it is
       associated with a parent object.  The generated line will be infinite.
       Defined with a point along the line and a direction vector.
@since SketchUp 2016, API 4.0
@param[in]  guide_line The guide line object.
@param[out] point      A point on the guide line.
@param[out] direction  The guide line direction vector.
@related SUGuideLineRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if guide_line is NULL
- \ref SU_ERROR_OVERWRITE_VALID if guide_line references a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if start or end is NULL
*/
SU_RESULT SUGuideLineCreateInfinite(
    SUGuideLineRef* guide_line, const struct SUPoint3D* point, const struct SUVector3D* direction);

/**
@brief Releases a guide line object. The guide line object must have been
       created with \ref SUGuideLineCreateFinite() or \ref
       SUGuideLineCreateInfinite and not subsequently associated with a parent
       object (e.g. \ref SUEntitiesRef).
@since SketchUp 2016, API 4.0
@param[in] guide_line The guide line object.
@related SUGuideLineRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if guide_line is NULL
- \ref SU_ERROR_INVALID_INPUT if guide_line does not reference a valid object
*/
SU_RESULT SUGuideLineRelease(SUGuideLineRef* guide_line);

/**
@brief Retrieves the data defining the line (a point, a direction vector, and a
       boolean flagging if the line is infinite).  For finite lines \p start
       is the start point, and the end point can be obtained by adding
       the direction vector (\p direction) to the start point (\p start).
       For infinite lines \p start is simply a point on the guide line, and
       \p direction is always a unit vector.
@since SketchUp 2016, API 4.0
@param[in]  guide_line The guide line object.
@param[out] start      A point on the guide line.
@param[out] direction  The guide line direction.
@param[out] isinfinite returns true if infinite otherwise returns false
@related SUGuideLineRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if guide line is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if start, direction, or isinfinite is NULL
*/
SU_RESULT SUGuideLineGetData(
    SUGuideLineRef guide_line, struct SUPoint3D* start, struct SUVector3D* direction,
    bool* isinfinite);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_GUIDE_LINE_H_
