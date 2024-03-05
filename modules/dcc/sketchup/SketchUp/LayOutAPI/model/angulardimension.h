// Copyright 2016 Trimble Navigation Ltd. All Rights Reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_ANGULARDIMENSION_H_
#define LAYOUT_MODEL_ANGULARDIMENSION_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/geometry/geometry.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOAngularDimensionRef
@brief References an angular dimension entity. An angular dimension is composed
       of the following sub-entities:
       - two 'extension lines' that extend from the entity being dimensioned.
       - a 'dimension line' connecting the ends of the leaders.
         (this may be represented by one or two paths depending on appearance.)
       - an optional 'leader line' that is used for small dimensions.
       - a 'dimension text' that displays the dimension's text.

       There are seven points that may be modified for an angular dimension:
       - two 'connection points' that define the start and end of the
         dimension.
       - two 'extent points' that define the start and end of the dimension
         line and are the ends of the two extension lines.
       - two 'offset points' that define the starting points of the extension
         lines.
       - one 'arc center point' that defines the center of the dimension, where
         the extension lines intersect.
@since LayOut 2017, API 2.0
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@enum LOAngularDimensionLeaderLineType
@brief Defines the different types of leader lines for an angular dimension
       entity.
@since LayOut 2017, API 2.0
*/
typedef enum {
  LOAngularDimensionLeaderLineType_SingleSegment = 0,  ///< Single segment leader line.
  LOAngularDimensionLeaderLineType_TwoSegment,         ///< Two segment leader line.
  LOAngularDimensionLeaderLineType_Bezier,             ///< Curved Bezier leader line.
  LOAngularDimensionLeaderLineType_Hidden,             ///< No visible leader line.
  LONumAngularDimensionLeaderLineTypes
} LOAngularDimensionLeaderLineType;

/**
@brief Creates a new disconnected angular dimension object. The arc center point
       is defined implicitly by the intersection of the two extension lines, and
       the radius is defined implicitly by the distance of the extent points
       from the arc center point. If the extent points are not equidistant from
       the arc center point, then the end extent point is adjusted
       automatically.
@since LayOut 2017, API 2.0
@param[out] dimension          The angular dimension object
@param[in]  start_point        Where the dimension should start
@param[in]  end_point          Where the dimension should end
@param[in]  start_extent_point The extent point where the dimension line should
                               start
@param[in]  end_extent_point   The extent point where the dimension line should
                               end
@param[in]  inner_angle        Whether or not the dimension should measure the
                               inner angle. If false, it will measure the outer
                               angle.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if dimension is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *dimension already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if start_point is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if end_point is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if start_extent_point is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if end_extent_point is NULL
*/
LO_RESULT LOAngularDimensionCreate(
    LOAngularDimensionRef* dimension, const LOPoint2D* start_point, const LOPoint2D* end_point,
    const LOPoint2D* start_extent_point, const LOPoint2D* end_extent_point, bool inner_angle);

/**
@brief Adds a reference to an angular dimension object.
@since LayOut 2017, API 2.0
@param[in] dimension The angular dimension object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
*/
LO_RESULT LOAngularDimensionAddReference(LOAngularDimensionRef dimension);

/**
@brief Releases an angular dimension object. The object will be invalidated if
       releasing the last reference.
@since LayOut 2017, API 2.0
@param[in] dimension The angular dimension object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if dimension is NULL
- \ref SU_ERROR_INVALID_INPUT if *dimension does not refer to a valid object
*/
LO_RESULT LOAngularDimensionRelease(LOAngularDimensionRef* dimension);

/**
@brief Converts from a \ref LOEntityRef to a \ref LOAngularDimensionRef.
       This is essentially a downcast operation so the given \ref LOEntityRef
       must be convertible to a \ref LOAngularDimensionRef.
@since LayOut 2017, API 2.0
@param[in] entity The entity object.
@return
- The converted \ref LOAngularDimensionRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
LO_EXPORT LOAngularDimensionRef LOAngularDimensionFromEntity(LOEntityRef entity);

/**
@brief Converts from a \ref LOAngularDimensionRef to a \ref LOEntityRef.
       This is essentially an upcast operation.
@since LayOut 2017, API 2.0
@param[in] dimension The angular dimension object.
@return
- The converted \ref LOEntityRef if dimension is a valid object
- If not, the returned reference will be invalid
*/
LO_EXPORT LOEntityRef LOAngularDimensionToEntity(LOAngularDimensionRef dimension);

/**
@brief Creates the entities that represent the dimension in its exploded form
       and adds them to a \ref LOEntityListRef. It is NOT necessary to
       explicitly release these entities, since \ref LOEntityListRef itself
       adds a reference to the entities and will release them when they are
       removed from the list or when the list is released. Depending on the
       appearance of the dimension being exploded, this may return anywhere from
       four to six entities. Start extension line, end extension line,
       one or two dimension lines, dimension text, optional leader line.
@since LayOut 2017, API 2.0
@param[in] dimension The angular dimension object.
@param[in] entity_list The entity list object to add the new entities to.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if entity_list does not refer to a valid object
*/
LO_RESULT LOAngularDimensionGetExplodedEntities(
    LOAngularDimensionRef dimension, LOEntityListRef entity_list);

/**
@brief Gets whether or not the dimension text uses custom text. When
       true, the dimension text will display a custom string that doesn't
       change. When false, the dimension text will display the length
       measurement and will update automatically.
@since LayOut 2017, API 2.0
@param[in]  dimension       The angular dimension object.
@param[out] use_custom_text Whether the dimension uses custom text.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if use_auto_text is NULL
*/
LO_RESULT LOAngularDimensionGetUsesCustomText(
    LOAngularDimensionRef dimension, bool* use_custom_text);

/**
@brief Sets whether or not the dimension text uses custom text. When true, the
       dimension text will display a custom string that doesn't change. When
       false, the dimension text will display the length measurement and will
       update automatically.
@since LayOut 2017, API 2.0
@param[in] dimension       The angular dimension object.
@param[in] use_custom_text Whether or not the dimension should use custom text.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOAngularDimensionSetUsesCustomText(
    LOAngularDimensionRef dimension, bool use_custom_text);

/**
@brief Creates a copy of the dimension text's \ref LOFormattedTextRef object.
@note With the addition of auto-text in dimensions for LayOut 2019.2, the
      copy of the dimension text incorrectly provided the plain text when
      requesting the display text. This has been fixed in LayOut 2020.1, API 5.1.
@since LayOut 2017, API 2.0
@param[in]  dimension The angular dimension object.
@param[out] text      The formatted text object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if text is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *text already refers to a valid object
*/
LO_RESULT LOAngularDimensionCreateDimensionTextCopy(
    LOAngularDimensionRef dimension, LOFormattedTextRef* text);

/**
@brief Sets the text of the dimension from a \ref LOFormattedTextRef object.
       The dimension lines may be adjusted if the text has been re-positioned.
@since LayOut 2017, API 2.0
@param[in] dimension The angular dimension object.
@param[in] text      The formatted text object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOAngularDimensionSetDimensionText(
    LOAngularDimensionRef dimension, LOFormattedTextRef text);

/**
@brief Gets the type of leader line the angular dimension is using.
@since LayOut 2017, API 2.0
@param[in]  dimension        The angular dimension object.
@param[out] leader_line_type The leader line type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if leader_line_type is NULL
*/
LO_RESULT LOAngularDimensionGetLeaderLineType(
    LOAngularDimensionRef dimension, LOAngularDimensionLeaderLineType* leader_line_type);

/**
@brief Sets the type of leader line the angular dimension is using. New control
       points will be generated if changing to a two segment or bezier leader
       line type. Setting the leader line type to \ref
       LOAngularDimensionLeaderLineType_Hidden will not move the dimension text.
@since LayOut 2017, API 2.0
@param[in] dimension        The angular dimension object.
@param[in] leader_line_type The leader line type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if leader_line_type is not a valid value
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOAngularDimensionSetLeaderLineType(
    LOAngularDimensionRef dimension, LOAngularDimensionLeaderLineType leader_line_type);

/**
@brief Gets the page location for the first connection.
@since LayOut 2017, API 2.0
@param[in]  dimension The angular dimension object.
@param[out] point     The position of the first connection point.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LOAngularDimensionGetStartConnectionPoint(
    LOAngularDimensionRef dimension, LOPoint2D* point);

/**
@brief Sets the page location for the first connection.
@since LayOut 2017, API 2.0
@param[in] dimension The angular dimension object.
@param[in] point     The position for the first connection point.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOAngularDimensionSetStartConnectionPoint(
    LOAngularDimensionRef dimension, const LOPoint2D* point);

/**
@brief Gets the page location for the second connection.
@since LayOut 2017, API 2.0
@param[in]  dimension The angular dimension object.
@param[out] point     The position of the second control point.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LOAngularDimensionGetEndConnectionPoint(
    LOAngularDimensionRef dimension, LOPoint2D* point);

/**
@brief Sets the page location for the second connection.
@since LayOut 2017, API 2.0
@param[in] dimension The angular dimension object.
@param[in] point     The position for the second control point.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOAngularDimensionSetEndConnectionPoint(
    LOAngularDimensionRef dimension, const LOPoint2D* point);

/**
@brief Gets the page location for the start of the dimension line.
@since LayOut 2017, API 2.0
@param[in]  dimension The angular dimension object.
@param[out] point     The position of the start of the dimension line.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LOAngularDimensionGetStartExtentPoint(LOAngularDimensionRef dimension, LOPoint2D* point);

/**
@brief Sets the page location for the start of the dimension line. This will
       also adjust the end of the dimension line to maintain the correct
       dimension line length.
@since LayOut 2017, API 2.0
@param[in] dimension The angular dimension object.
@param[in] point The position of the start of the dimension line.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOAngularDimensionSetStartExtentPoint(
    LOAngularDimensionRef dimension, const LOPoint2D* point);

/**
@brief Gets the page location for the end of the dimension line.
@since LayOut 2017, API 2.0
@param[in]  dimension The angular dimension object.
@param[out] point     The position of the end of the dimension line.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LOAngularDimensionGetEndExtentPoint(LOAngularDimensionRef dimension, LOPoint2D* point);

/**
@brief Sets the page location for the end of the dimension line. This will
       also adjust the start of the dimension line to maintain the correct
       dimension line length.
@since LayOut 2017, API 2.0
@param[in] dimension The angular dimension object.
@param[in] point     The position of the end of the dimension line.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOAngularDimensionSetEndExtentPoint(
    LOAngularDimensionRef dimension, const LOPoint2D* point);

/**
@brief Gets the page location for the start of the first extension line. The
       first extension line runs from this offset point to the start extent
       point.
@since LayOut 2017, API 2.0
@param[in]  dimension The angular dimension object.
@param[out] point     The position of the start of the extension line.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LOAngularDimensionGetStartOffsetPoint(LOAngularDimensionRef dimension, LOPoint2D* point);

/**
@brief Gets the page location for the start of the second extension line. The
       second extension line runs from this offset point to the end extent
       point.
@since LayOut 2017, API 2.0
@param[in]  dimension The angular dimension object.
@param[out] point     The position of the end of the dimension line.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LOAngularDimensionGetEndOffsetPoint(LOAngularDimensionRef dimension, LOPoint2D* point);

/**
@brief Sets the length of the offset from the first connection point to the
       start of the first extension line. The connection and extent points will
       not move.
@since LayOut 2017, API 2.0
@param[in] dimension The angular dimension object.
@param[in] length    The length the offset to the extension line should be.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOAngularDimensionSetStartOffsetLength(LOAngularDimensionRef dimension, double length);

/**
@brief Sets the length of the offset from the second connection point to the
       start of the second extension line. The connection and extent points
       will not move.
@since LayOut 2017, API 2.0
@param[in] dimension The angular dimension object.
@param[in] length    The length the offset to the extension line should be.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOAngularDimensionSetEndOffsetLength(LOAngularDimensionRef dimension, double length);

/**
@brief Gets the page location for the dimension arc center point.
@since LayOut 2017, API 2.0
@param[in]  dimension The angular dimension object.
@param[out] point     The position of the dimension arc center point.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LOAngularDimensionGetArcCenterPoint(LOAngularDimensionRef dimension, LOPoint2D* point);

/**
@brief Gets the angular dimension's radius. This is the distance from the arc
       center point to the dimension line.
@since LayOut 2017, API 2.0
@param[in]  dimension The angular dimension object.
@param[out] radius    The dimension radius.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if radius is NULL
*/
LO_RESULT LOAngularDimensionGetRadius(LOAngularDimensionRef dimension, double* radius);

/**
@brief Sets the angular dimension's radius. This is the distance from the arc
       center point to the dimension line.
@since LayOut 2017, API 2.0
@param[in] dimension The angular dimension object.
@param[in] radius    The dimension radius.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
- \ref SU_ERROR_OUT_OF_RANGE if radius is less than or equal to zero
*/
LO_RESULT LOAngularDimensionSetRadius(LOAngularDimensionRef dimension, double radius);

/**
@brief Gets the angular dimension's angle. The angle is represented in radians.
@since LayOut 2017, API 2.0
@param[in]  dimension The angular dimension object.
@param[out] angle     The dimension angle.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if angle is NULL
*/
LO_RESULT LOAngularDimensionGetAngle(LOAngularDimensionRef dimension, double* angle);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_ANGULARDIMENSION_H_
