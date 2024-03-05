// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_ELLIPSE_H_
#define LAYOUT_MODEL_ELLIPSE_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/geometry/geometry.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOEllipseRef
@brief References a simple elliptical shape entity.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Creates a new ellipse entity object.
@param[out] ellipse The newly created ellipse.
@param[in]  bounds  The bounds of the ellipse.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if ellipse is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *ellipse already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if bounds is NULL
- \ref SU_ERROR_OUT_OF_RANGE if bounds are not greater than (0, 0)
*/
LO_RESULT LOEllipseCreate(LOEllipseRef* ellipse, const LOAxisAlignedRect2D* bounds);

/**
@brief Adds a reference to an ellipse object.
@param[in] ellipse The ellipse object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if ellipse does not refer to a valid object
*/
LO_RESULT LOEllipseAddReference(LOEllipseRef ellipse);

/**
@brief Releases an ellipse object. The object will be invalidated if
       releasing the last reference.
@param[in] ellipse The ellipse object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if ellipse is NULL
- \ref SU_ERROR_INVALID_INPUT if *ellipse does not refer to a valid object
*/
LO_RESULT LOEllipseRelease(LOEllipseRef* ellipse);

/**
@brief Converts from a \ref LOEntityRef to a \ref LOEllipseRef.
       This is essentially a downcast operation so the given \ref LOEntityRef
       must be convertible to a \ref LOEllipseRef.
@param[in] entity The entity object.
@return
- The converted \ref LOEllipseRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
LO_EXPORT LOEllipseRef LOEllipseFromEntity(LOEntityRef entity);

/**
@brief Converts from a \ref LOEllipseRef to a \ref LOEntityRef.
       This is essentially an upcast operation.
@param[in] ellipse The ellipse object.
@return
- The converted \ref LOEntityRef if ellipse is a valid object
- If not, the returned reference will be invalid
*/
LO_EXPORT LOEntityRef LOEllipseToEntity(LOEllipseRef ellipse);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_ELLIPSE_H_
