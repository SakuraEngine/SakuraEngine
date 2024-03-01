// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_RECTANGLE_H_
#define LAYOUT_MODEL_RECTANGLE_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/geometry/geometry.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LORectangleRef
@brief References a simple rectangular shape entity.
*/

/**
@enum LORectangleType
@brief Defines the shape of the rectangle within its boundaries.
*/
typedef enum {
  LORectangleType_Normal = 0,  ///< Normal rectangle with 90 degree corners.
  LORectangleType_Rounded,     ///< Rectangle with rounded corners and a specified radius.
  LORectangleType_Lozenge,     ///< Rectangle with rounded corners and a radius equal to half the
                               ///< smaller of width or height.
  LORectangleType_Bulged,      ///< Rectangle whose left and right sides are arcs with a specified
                               ///< radius.
  LONumRectangleTypes
} LORectangleType;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Creates a 'normal' rectangle with 90 degree corners.
@param[out] rectangle The rectangle object.
@param[in]  bounds    The starting dimensions of the rectangle.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if rectangle is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *rectangle already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if bounds is NULL
- \ref SU_ERROR_OUT_OF_RANGE if bounds has a width or height of zero
*/
LO_RESULT LORectangleCreate(LORectangleRef* rectangle, const LOAxisAlignedRect2D* bounds);

/**
@brief Creates a 'rounded' rectangle with arcs in each corner.
@param[out] rectangle     The rectangle object.
@param[in]  bounds        The starting dimensions of the rectangle.
@param[in]  corner_radius The radius of the circles in the corners in inches.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if rectangle is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *rectangle already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if bounds is NULL
- \ref SU_ERROR_OUT_OF_RANGE if bounds has a width or height of zero
- \ref SU_ERROR_OUT_OF_RANGE if corner_radius is negative
*/
LO_RESULT LORectangleCreateRounded(
    LORectangleRef* rectangle, const LOAxisAlignedRect2D* bounds, double corner_radius);
/**
@brief Creates a 'Lozenge' rectangle with the shorter side curved.
@param[out] rectangle The rectangle object.
@param[in]  bounds    The starting dimensions of the rectangle.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if rectangle is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *rectangle already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if bounds is NULL
- \ref SU_ERROR_OUT_OF_RANGE if bounds has a width or height of zero
*/
LO_RESULT LORectangleCreateLozenge(LORectangleRef* rectangle, const LOAxisAlignedRect2D* bounds);
/**
@brief Creates a 'bulged' rectangle with the vertical sides curved.
@param[out] rectangle      The rectangle object.
@param[in]  bounds         The starting dimensions of the rectangle.
@param[in]  bulge_distance The size of the bulge, from the outer edge of the
                           rectangle in.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if rectangle is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *rectangle already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if bounds is NULL
- \ref SU_ERROR_OUT_OF_RANGE if bounds has a width or height of zero
- \ref SU_ERROR_OUT_OF_RANGE if bulge_distance is negative
*/
LO_RESULT LORectangleCreateBulged(
    LORectangleRef* rectangle, const LOAxisAlignedRect2D* bounds, double bulge_distance);

/**
@brief Adds a reference to a rectangle object.
@param[in] rectangle The rectangle object.
@return
SU_ERROR_NONE on success
SU_ERROR_INVALID_INPUT if rectangle does not refer to a valid object
*/
LO_RESULT LORectangleAddReference(LORectangleRef rectangle);

/**
@brief Releases a rectangle object. The object will be invalidated if releasing
       the last reference.
@param[in] rectangle The rectangle object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if rectangle is NULL
- \ref SU_ERROR_INVALID_INPUT if *rectangle does not refer to a valid object
*/
LO_RESULT LORectangleRelease(LORectangleRef* rectangle);

/**
@brief Converts from a \ref LOEntityRef to a \ref LORectangleRef.
       This is essentially a downcast operation so the given \ref LOEntityRef
       must be convertible to a \ref LORectangleRef.
@param[in] entity The entity object.
@return
- The converted \ref LORectangleRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
LO_EXPORT LORectangleRef LORectangleFromEntity(LOEntityRef entity);

/**
@brief Converts from a \ref LORectangleRef to a \ref LOEntityRef.
       This is essentially an upcast operation.
@param[in] rectangle The rectangle object.
@return
- The converted \ref LOEntityRef if rectangle is a valid object
- If not, the returned reference will be invalid
*/
LO_EXPORT LOEntityRef LORectangleToEntity(LORectangleRef rectangle);

/**
@brief Gets the point that was originally defined as the upper left point.
       Transforms to this object may have moved it so that it is no longer the
       top left point geometrically on the page.
@param[in]  rectangle The rectangle object.
@param[out] point     The position of the original top left point.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if rectangle does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LORectangleGetTopLeftPoint(LORectangleRef rectangle, LOPoint2D* point);

/**
@brief Gets the point that was originally defined as the upper right point.
       Transforms to this object may have moved it so that it is no longer the
       top right point geometrically on the page.
@param[in]  rectangle The rectangle object.
@param[out] point     The position of the original top right point.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if rectangle does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LORectangleGetTopRightPoint(LORectangleRef rectangle, LOPoint2D* point);

/**
@brief Gets the point that was originally defined as the bottom left point.
       Transforms to this object may have moved it so that it is no longer the
       bottom left point geometrically on the page.
@param[in]  rectangle The rectangle object.
@param[out] point     The position of the original bottom left point.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if rectangle does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LORectangleGetBottomLeftPoint(LORectangleRef rectangle, LOPoint2D* point);

/**
@brief Gets the point that was originally defined as the bottom right point.
       Transforms to this object may have moved it so that it is no longer the
       bottom right point geometrically on the page.
@param[in]  rectangle The rectangle object.
@param[out] point     The position of the original bottom right point.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if rectangle does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LORectangleGetBottomRightPoint(LORectangleRef rectangle, LOPoint2D* point);

/**
@brief Gets the type of a rectangle.
@param[in]  rectangle The rectangle object.
@param[out] type      The type of the rectangle.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if rectangle does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if type is NULL
*/
LO_RESULT LORectangleGetRectangleType(LORectangleRef rectangle, LORectangleType* type);

/**
@brief Gets the type of a rectangle.
@param[in] rectangle The rectangle object.
@param[in] type      The type of the rectangle.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if rectangle does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if rectangle is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if rectangle is locked
- \ref SU_ERROR_GENERIC if type is not a valid rectangle type
*/
LO_RESULT LORectangleSetRectangleType(LORectangleRef rectangle, LORectangleType type);

/**
@brief Returns the radius that defines the shape of a bulged or rounded
       rectangle.
@param[in]  rectangle The rectangle object.
@param[out] radius    The radius for bulged and rounded rectangles.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if rectangle does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if radius is NULL
- \ref SU_ERROR_NO_DATA if the rectangle is not of type \ref
  LORectangleType_Bulged or \ref LORectangleType_Rounded
*/
LO_RESULT LORectangleGetRadius(LORectangleRef rectangle, double* radius);

/**
@brief Sets the radius that defines the shape of a bulged or rounded rectangle.
@param[in]  rectangle The rectangle object.
@param[out] radius    The radius for bulged and rounded rectangles.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if rectangle does not refer to a valid object
- \ref SU_ERROR_NO_DATA if the rectangle is not of type \ref
  LORectangleType_Bulged or \ref LORectangleType_Rounded
- \ref SU_ERROR_OUT_OF_RANGE if radius is negative
- \ref SU_ERROR_LAYER_LOCKED if rectangle is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if rectangle is locked
*/
LO_RESULT LORectangleSetRadius(LORectangleRef rectangle, double radius);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus
#endif  // LAYOUT_MODEL_RECTANGLE_H_
