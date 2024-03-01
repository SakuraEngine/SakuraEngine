// Copyright 2016 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUDimensionLinearRef.
 */
#ifndef SKETCHUP_MODEL_DIMENSION_LINEAR_H_
#define SKETCHUP_MODEL_DIMENSION_LINEAR_H_

#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUDimensionLinearRef
@extends SUDimensionRef
@brief  A linear dimension entity reference.
@since SketchUp 2017, API 5.0
*/

/**
@enum SUHorizontalTextPositionType
@brief Indicates the different supported horizontal text position types
@since SketchUp 2017, API 5.0
*/
enum SUHorizontalTextPositionType {
  SUHorizontalTextPositionCenter = 0,
  SUHorizontalTextPositionOutsideStart,
  SUHorizontalTextPositionOutsideEnd
};

/**
@enum SUVerticalTextPositionType
@brief Indicates the different supported horizontal text position types
@since SketchUp 2017, API 5.0
*/
enum SUVerticalTextPositionType {
  SUVerticalTextPositionCenter = 0,
  SUVerticalTextPositionAbove,
  SUVerticalTextPositionBelow
};

/**
@enum SUDimensionLinearAlignmentType
@brief Indicates the different supported horizontal text position types
@since SketchUp 2019, API 7.0
*/
enum SUDimensionLinearAlignmentType {
  SUDimensionLinearAlignmentAligned = 0,
  SUDimensionLinearAlignmentVertical,
  SUDimensionLinearAlignmentHorizontal
};

/**
@brief Converts from an \ref SUDimensionLinearRef to an \ref SUDimensionRef.
       This is essentially an upcast operation.
@since SketchUp 2017, API 5.0
@param[in] dimension The given dimension reference.
@related SUDimensionLinearRef
@return
- The converted \ref SUDimensionRef if dimension is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDimensionRef SUDimensionLinearToDimension(SUDimensionLinearRef dimension);

/**
@brief Converts from an \ref SUDimensionRef to an \ref SUDimensionLinearRef.
       This is essentially a downcast operation so the given \ref
       SUDimensionRef must be convertible to an \ref SUDimensionLinearRef.
@since SketchUp 2017, API 5.0
@param[in] dimension The given dimension reference.
@related SUDimensionLinearRef
@return
- The converted \ref SUDimensionLinearRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDimensionLinearRef SUDimensionLinearFromDimension(SUDimensionRef dimension);

/**
@brief Creates a new linear dimension object with default data. Refer to the
       documentation for \ref SUDimensionLinearSetStartPoint() for more
       information about the various supported ways for setting connection
       points.
@since SketchUp 2017, API 5.0
@param[out] dimension   The dimension object created.
@param[in]  start_point The 3d point used to set the start location.
@param[in]  start_path  The instance path used to specify the entity connected
            to the dimension's start.
@param[in]  end_point   The 3d point used to set the end location.
@param[in]  end_path    The instance path used to specify the entity connected
            to the dimension's end.
@param[in]  offset      The offset distance from the measured entities to be
            set.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if dimension is NULL
- \ref SU_ERROR_OVERWRITE_VALID if dimension already references a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if {start,end}_path is invalid and
       {start,end}_point is NULL
- \ref SU_ERROR_INVALID_ARGUMENT {start,end}_path is valid but refers to an
       invalid instance path
- \ref SU_ERROR_GENERIC if {start,end}_point is NULL and {start,end}_path
       doesn't have a vertex or guide point for its leaf
*/
SU_RESULT SUDimensionLinearCreate(
    SUDimensionLinearRef* dimension, const struct SUPoint3D* start_point,
    SUInstancePathRef start_path, const struct SUPoint3D* end_point, SUInstancePathRef end_path,
    double offset);

/**
@brief Releases a dimension object.
@since SketchUp 2017, API 5.0
@param[in] dimension The dimension object.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if dimension is NULL
*/
SU_RESULT SUDimensionLinearRelease(SUDimensionLinearRef* dimension);

/**
@brief Retrieves the start point of a dimension object. The given instance path
       object either must have been constructed using one of the
       SUInstancePathCreate* functions or it will be generated on the fly if it
       is invalid. It must be released using SUInstancePathRelease() when
       it is no longer needed.
@since SketchUp 2017, API 5.0
@param[in]  dimension The dimension object.
@param[out] point     The 3d point retrieved.
@param[out] path      The instance path retrieved.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point or path are NULL
*/
SU_RESULT SUDimensionLinearGetStartPoint(
    SUDimensionLinearRef dimension, struct SUPoint3D* point, SUInstancePathRef* path);

/**
@brief Sets the start connection point of a dimension object. A dimension's
       connection point can be set in a few different ways. In the simplest
       form a connection point can be set to an arbitrary point in space by
       providing a non-null \ref SUPoint3D and an invalid
       \ref SUInstancePathRef. The more complex forms connect the point to a
       position on an entity in the model by  providing a valid \ref
       SUInstancePathRef which refers to an existing model entity. In the more
       complex forms the the input SUPoint3D must be non-null for all
       connectable entity types except for vertices and guide points, in which
       case the \ref SUPoint3D argument may be null as it will be ignored. It
       should be noted that when changing a dimension's connection point the
       other point may need to be adjusted as well. Users may want to verify
       the other connection point after setting this one.
@since SketchUp 2017, API 5.0

@code{.c}
  // Simple Example: Connect to an arbitrary position in space
  SUPoint3D point{ xposition, yposition, zposition};
  SUDimensionLinearSetStartPoint(dimension, &point, SU_INVALID);
@endcode

@code{.c}
  // Vertex Example: Connect to vertex entity
  SUInstancePathRef path = SU_INVALID;
  SUInstancePathCreate(&path);
  SUInstancePathSetLeaf(path, SUVertexToEntity(vertex));
  SUDimensionLinearSetStartPoint(dimension, NULL, path);
@endcode

@code{.c}
  // Edge Example: Connect to nearest point on an instance of an edge entity
  SUPoint3D point{ xposition, yposition, zposition};
  SUInstancePathRef path = SU_INVALID;
  SUInstancePathCreate(&path);
  SUInstancePathPushInstance(path, instance);
  SUInstancePathSetLeaf(path, SUEdgeToEntity(edge));
  SUDimensionLinearSetStartPoint(dimension, &point, path);
@endcode

@param[in] dimension The dimension object.
@param[in] point     The 3d point to be set.
@param[in] path      The instance path to be set.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if path is invalid and point is NULL
- \ref SU_ERROR_INVALID_ARGUMENT path is valid but refers to an invalid
       instance path
- \ref SU_ERROR_GENERIC if point is NULL and path doesn't have a vertex or
       guide point for its leaf
*/
SU_RESULT SUDimensionLinearSetStartPoint(
    SUDimensionLinearRef dimension, const struct SUPoint3D* point, SUInstancePathRef path);

/**
@brief Retrieves the end point of a dimension object. The given instance path
       object either must have been constructed using one of the
       SUInstancePathCreate* functions or it will be generated on the fly if it
       is invalid. It must be released using SUInstancePathRelease() when
       it is no longer needed.
@since SketchUp 2017, API 5.0
@param[in]  dimension The dimension object.
@param[out] point     The 3d point retrieved.
@param[out] path      The instance path retrieved.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point or path are NULL
*/
SU_RESULT SUDimensionLinearGetEndPoint(
    SUDimensionLinearRef dimension, struct SUPoint3D* point, SUInstancePathRef* path);

/**
@brief Sets the end connection point of a dimension object. Refer to the
       documentation for \ref SUDimensionLinearSetStartPoint() for a detailed
       description on supported ways of setting a dimension's connection point.
@since SketchUp 2017, API 5.0
@param[in] dimension The dimension object.
@param[in] point     The 3d point to be set.
@param[in] path      The instance path to be set.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if path is invalid and point is NULL
- \ref SU_ERROR_INVALID_ARGUMENT path is valid but refers to an invalid
       instance path
- \ref SU_ERROR_GENERIC if point is NULL and path doesn't have a vertex or
       guide point for its leaf
*/
SU_RESULT SUDimensionLinearSetEndPoint(
    SUDimensionLinearRef dimension, const struct SUPoint3D* point, SUInstancePathRef path);

/**
@brief Retrieves the x-axis of a dimension object. The x-axis is the axis along
       the length of the dimension.
@since SketchUp 2017, API 5.0
@param[in]  dimension The dimension object.
@param[out] axis      The 3d vector retrieved.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if path is invalid and point is NULL
- \ref SU_ERROR_INVALID_INPUT if point is not NULL and path refers to an
       invalid instance path
- \ref SU_ERROR_GENERIC if point is NULL and path doesn't have a vertex or
       guide point for its leaf
*/
SU_RESULT SUDimensionLinearGetXAxis(SUDimensionLinearRef dimension, struct SUVector3D* axis);

/**
@brief Sets the x-axis of a dimension object.
@since SketchUp 2017, API 5.0
@param[in] dimension The dimension object.
@param[in] axis      The 3d vector to be set.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if axis is NULL
*/
SU_RESULT SUDimensionLinearSetXAxis(SUDimensionLinearRef dimension, const struct SUVector3D* axis);

/**
@brief Retrieves the normal vector of a dimension object. The normal vector is
       a unit vector pointing out of the plane of the linear dimension. A
       linear dimension's plane is the plane defined by the x-axis and the
       leader lines' direction vector.
@since SketchUp 2017, API 5.0
@param[in]  dimension The dimension object.
@param[out] normal    The 3d vector retrieved.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if normal is NULL
*/
SU_RESULT SUDimensionLinearGetNormal(SUDimensionLinearRef dimension, struct SUVector3D* normal);

/**
@brief Sets the normal vector of a dimension object.
@since SketchUp 2017, API 5.0
@param[in] dimension The dimension object.
@param[in] normal    The 3d vector to be set.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if normal is NULL
*/
SU_RESULT SUDimensionLinearSetNormal(
    SUDimensionLinearRef dimension, const struct SUVector3D* normal);

/**
@brief Retrieves the position of a dimension object. The position is a 2D point
       in the dimension's plane indicating where the dimension text is located.
@since SketchUp 2017, API 5.0
@param[in]  dimension The dimension object.
@param[out] position  The position retrieved.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if position is NULL
*/
SU_RESULT SUDimensionLinearGetPosition(SUDimensionLinearRef dimension, struct SUPoint2D* position);

/**
@brief Sets the position of a dimension object.
@since SketchUp 2017, API 5.0
@param[in] dimension The dimension object.
@param[in] position  The position to be set.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if position is NULL
*/
SU_RESULT SUDimensionLinearSetPosition(
    SUDimensionLinearRef dimension, const struct SUPoint2D* position);

/**
@brief Retrieves an enum value indicating the dimension's vertical alignment
       type.
@since SketchUp 2017, API 5.0
@param[in]  dimension The dimension object.
@param[out] alignment The dimension alignment enum value retrieved.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if alignment is NULL
*/
SU_RESULT SUDimensionLinearGetVerticalAlignment(
    SUDimensionLinearRef dimension, enum SUVerticalTextPositionType* alignment);

/**
@brief Sets the dimension's vertical alignment type.
@since SketchUp 2017, API 5.0
@param[in] dimension The dimension object.
@param[in] alignment The dimension alignment type to be set.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
*/
SU_RESULT SUDimensionLinearSetVerticalAlignment(
    SUDimensionLinearRef dimension, enum SUVerticalTextPositionType alignment);

/**
@brief Retrieves an enum value indicating the dimension's horizontal alignment
       type.
@since SketchUp 2017, API 5.0
@param[in]  dimension The dimension object.
@param[out] alignment The dimension alignment enum value retrieved.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if alignment is NULL
*/
SU_RESULT SUDimensionLinearGetHorizontalAlignment(
    SUDimensionLinearRef dimension, enum SUHorizontalTextPositionType* alignment);

/**
@brief Sets the dimension's horizontal alignment type.
@since SketchUp 2017, API 5.0
@param[in] dimension The dimension object.
@param[in] alignment The dimension alignment type to be set.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
*/
SU_RESULT SUDimensionLinearSetHorizontalAlignment(
    SUDimensionLinearRef dimension, enum SUHorizontalTextPositionType alignment);

/**
@brief Retrieves an enum value indicating the linear dimension's alignment
type.
@since SketchUp 2019, API 7.0
@param[in]  dimension The dimension object.
@param[out] alignment The dimension alignment enum value retrieved.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if alignment is NULL
*/
SU_RESULT SUDimensionLinearGetAlignment(
    SUDimensionLinearRef dimension, enum SUDimensionLinearAlignmentType* alignment);

/**
@brief Retrieves the position of the text location attachment point of the
       dimension text. Note that depending on the TextPosition enumerator, this
       can be the center or side of a text element.
@since SketchUp 2019, API 7.0
@param[in]  dimension The dimension object.
@param[out] position The position of the text element.
@related SUDimensionLinearRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_OUT_OF_RANGE if the dimension text relation is invalid
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if position is NULL
*/
SU_RESULT SUDimensionLinearGetTextPosition(
    SUDimensionLinearRef dimension, struct SUPoint3D* position);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_DIMENSION_LINEAR_H_
