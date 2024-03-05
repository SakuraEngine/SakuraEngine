// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_LINEARDIMENSION_H_
#define LAYOUT_MODEL_LINEARDIMENSION_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/geometry/geometry.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOLinearDimensionRef
@brief References a linear dimension entity. A linear dimension is composed of
       the following sub-entities:
       - two 'extension lines' that connect to the entity being dimensioned.
       - a 'dimension line' connecting the ends of the leaders.
         (this may be represented by one or two paths depending on appearance.)
       - an optional 'leader line' that is used for small dimensions.
       - a 'dimension text' that displays the dimension's text.

       There are six points that may be modified for a linear dimension:
       - two 'connection points' that define the start and end of the
         dimension.
       - two 'extent points' that define the start and end of the dimension
         line and are the ends of the two extension lines.
       - two 'offset points' that define the starting points of the extension
         lines.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@enum LOLinearDimensionLeaderLineType
@brief Defines the different types of leader lines for a linear dimension entity.
*/
typedef enum {
  LOLinearDimensionLeaderLineType_SingleSegment = 0,  ///< Single segment leader line.
  LOLinearDimensionLeaderLineType_TwoSegment,         ///< Two segment leader line.
  LOLinearDimensionLeaderLineType_Bezier,             ///< Curved Bezier leader line.
  LOLinearDimensionLeaderLineType_Hidden,             ///< No visible leader line.
  LONumLinearDimensionLeaderLineTypes
} LOLinearDimensionLeaderLineType;

/**
@brief Creates a new disconnected linear dimension object.
@param[out] dimension   The linear dimension object
@param[in]  start_point Where the dimension should start
@param[in]  end_point   Where the dimension should end
@param[in]  height      Distance from the start and end points to the dimension
                        line
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if dimension is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *dimension already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if start_point is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if end_point is NULL
*/
LO_RESULT LOLinearDimensionCreate(
    LOLinearDimensionRef* dimension, const LOPoint2D* start_point, const LOPoint2D* end_point,
    double height);

/**
@brief Adds a reference to a linear dimension object.
@param[in] dimension The linear dimension object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
*/
LO_RESULT LOLinearDimensionAddReference(LOLinearDimensionRef dimension);

/**
@brief Releases a linear dimension object. The object will be invalidated if
       releasing the last reference.
@param[in] dimension The linear dimension object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if dimension is NULL
- \ref SU_ERROR_INVALID_INPUT if *dimension does not refer to a valid object
*/
LO_RESULT LOLinearDimensionRelease(LOLinearDimensionRef* dimension);

/**
@brief Converts from a \ref LOEntityRef to a \ref LOLinearDimensionRef.
       This is essentially a downcast operation so the given \ref LOEntityRef
       must be convertible to a \ref LOLinearDimensionRef.
@param[in] entity The entity object.
@return
- The converted \ref LOLinearDimensionRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
LO_EXPORT LOLinearDimensionRef LOLinearDimensionFromEntity(LOEntityRef entity);

/**
@brief Converts from a \ref LOLinearDimensionRef to a \ref LOEntityRef.
       This is essentially an upcast operation.
@param[in] dimension The linear dimension object.
@return
- The converted \ref LOEntityRef if dimension is a valid object
- If not, the returned reference will be invalid
*/
LO_EXPORT LOEntityRef LOLinearDimensionToEntity(LOLinearDimensionRef dimension);

/**
@brief Creates the entities that represent the dimension in its exploded form
       and adds them to a \ref LOEntityListRef. It is NOT necessary to
       explicitly release these entities, since \ref LOEntityListRef itself
       adds a reference to the entities and will release them when they are
       removed from the list or when the list is released. Depending on the
       appearance of the dimension being exploded, this may return anywhere from
       four to six entities. Start extension line, end extension line,
       one or two dimension lines, dimension text, optional leader line.
@param[in] dimension The linear dimension object.
@param[in] entity_list The entity list object to add the new entities to.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if entity_list does not refer to a valid object
*/
LO_RESULT LOLinearDimensionGetExplodedEntities(
    LOLinearDimensionRef dimension, LOEntityListRef entity_list);

/**
@brief Gets whether or not the dimension text uses custom text. When
       true, the dimension text will display a custom string that doesn't
       change. When false, the dimension text will display the length
       measurement and will update automatically.
@param[in]  dimension       The linear dimension object.
@param[out] use_custom_text Whether the dimension uses custom text.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if use_auto_text is NULL
*/
LO_RESULT LOLinearDimensionGetUsesCustomText(LOLinearDimensionRef dimension, bool* use_custom_text);

/**
@brief Sets whether or not the dimension text uses custom text. When true, the
       dimension text will display a custom string that doesn't change. When
       false, the dimension text will display the length measurement and will
       update automatically.
@param[in] dimension       The linear dimension object.
@param[in] use_custom_text Whether or not the dimension should use custom text.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOLinearDimensionSetUsesCustomText(LOLinearDimensionRef dimension, bool use_custom_text);

/**
@brief Creates a copy of the dimension text's \ref LOFormattedTextRef object.
@note With the addition of auto-text in dimensions for LayOut 2019.2, the
      copy of the dimension text incorrectly provided the plain text when
      requesting the display text. This has been fixed in LayOut 2020.1, API 5.1.
@param[in]  dimension The linear dimension object.
@param[out] text      The formatted text object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if text is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *text already refers to a valid object
*/
LO_RESULT LOLinearDimensionCreateDimensionTextCopy(
    LOLinearDimensionRef dimension, LOFormattedTextRef* text);

/**
@brief Sets the text of the dimension from a \ref LOFormattedTextRef object.
       The dimension lines may be adjusted if the text has been re-positioned.
@param[in] dimension The linear dimension object.
@param[in] text      The formatted text object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOLinearDimensionSetDimensionText(
    LOLinearDimensionRef dimension, LOFormattedTextRef text);

/**
@brief Gets the type of leader line the linear dimension is using.
@param[in]  dimension        The linear dimension object.
@param[out] leader_line_type The leader line type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if leader_line_type is NULL
*/
LO_RESULT LOLinearDimensionGetLeaderLineType(
    LOLinearDimensionRef dimension, LOLinearDimensionLeaderLineType* leader_line_type);

/**
@brief Sets the type of leader line the linear dimension is using. New control
       points will be generated if changing to a two segment or bezier leader
       line type. Setting the leader line type to \ref
       LOLinearDimensionLeaderLineType_Hidden will not move the dimension text.
@param[in] dimension        The linear dimension object.
@param[in] leader_line_type The leader line type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if leader_line_type is not a valid value
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOLinearDimensionSetLeaderLineType(
    LOLinearDimensionRef dimension, LOLinearDimensionLeaderLineType leader_line_type);

/**
@brief Gets whether or not the scale for the dimension length text is set
       automatically.
@param[in]  dimension      The linear dimension object.
@param[out] use_auto_scale Whether the dimension is using auto scale or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if use_auto_scale is NULL
*/
LO_RESULT LOLinearDimensionGetUsesAutoScale(LOLinearDimensionRef dimension, bool* use_auto_scale);

/**
@brief Sets whether or not the scale for the dimension length text is set
       automatically.
@param[in] dimension      The linear dimension object.
@param[in] use_auto_scale Whether the dimension should use auto scale or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
*/
LO_RESULT LOLinearDimensionSetUsesAutoScale(LOLinearDimensionRef dimension, bool use_auto_scale);

/**
@brief Gets the scale being used for the dimension length text.
@param[in]  dimension The linear dimension object.
@param[out] scale     The dimension text scale.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if scale is NULL
*/
LO_RESULT LOLinearDimensionGetScale(LOLinearDimensionRef dimension, double* scale);

/**
@brief Sets the scale being used for the dimension length text. This will also
       make the scale not use auto-scale.
@param[in] dimension The linear dimension object.
@param[in] scale     The new scale to use.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if scale is not positive or scale is greater than 1
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOLinearDimensionSetScale(LOLinearDimensionRef dimension, double scale);

/**
@brief Gets the page location for the first connection.
@param[in]  dimension The linear dimension object.
@param[out] point     The position of the first connection point.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LOLinearDimensionGetStartConnectionPoint(
    LOLinearDimensionRef dimension, LOPoint2D* point);

/**
@brief Sets the page location for the first connection. This will cause the
       dimension to become disconnected. To make a connected dimension, call
       LOLinearDimensionConnectTo.
@param[in] dimension The linear dimension object.
@param[in] point     The position for the first connection point.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOLinearDimensionSetStartConnectionPoint(
    LOLinearDimensionRef dimension, LOPoint2D* point);

/**
@brief Gets the page location for the second connection. This will cause the
       dimension to become disconnected. To make a connected dimension, call
       LOLinearDimensionConnectTo.
@param[in]  dimension The linear dimension object.
@param[out] point     The position of the second control point.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LOLinearDimensionGetEndConnectionPoint(LOLinearDimensionRef dimension, LOPoint2D* point);

/**
@brief Sets the page location for the second connection.
@param[in] dimension The linear dimension object.
@param[in] point     The position for the second control point.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOLinearDimensionSetEndConnectionPoint(LOLinearDimensionRef dimension, LOPoint2D* point);

/**
@brief Gets the page location for the start of the dimension line.
@param[in]  dimension The linear dimension object.
@param[out] point     The position of the start of the dimension line.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LOLinearDimensionGetStartExtentPoint(LOLinearDimensionRef dimension, LOPoint2D* point);

/**
@brief Sets the page location for the start of the dimension line. This will
       also adjust the end of the dimension line to maintain the correct
       dimension line length.
@param[in] dimension The linear dimension object.
@param[in] point The position of the start of the dimension line.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOLinearDimensionSetStartExtentPoint(LOLinearDimensionRef dimension, LOPoint2D* point);

/**
@brief Gets the page location for the end of the dimension line.
@param[in]  dimension The linear dimension object.
@param[out] point     The position of the end of the dimension line.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LOLinearDimensionGetEndExtentPoint(LOLinearDimensionRef dimension, LOPoint2D* point);

/**
@brief Sets the page location for the end of the dimension line. This will
       also adjust the start of the dimension line to maintain the correct
       dimension line length.
@param[in] dimension The linear dimension object.
@param[in] point     The position of the end of the dimension line.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOLinearDimensionSetEndExtentPoint(LOLinearDimensionRef dimension, LOPoint2D* point);

/**
@brief Gets the page location for the start of the first extension line. The
       first extension line runs from this offset point to the start extent
       point.
@param[in]  dimension The linear dimension object.
@param[out] point     The position of the start of the extension line.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LOLinearDimensionGetStartOffsetPoint(LOLinearDimensionRef dimension, LOPoint2D* point);

/**
@brief Gets the page location for the start of the second extension line. The
       second extension line runs from this offset point to the end extent
       point.
@param[in]  dimension The linear dimension object.
@param[out] point     The position of the end of the dimension line.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
LO_RESULT LOLinearDimensionGetEndOffsetPoint(LOLinearDimensionRef dimension, LOPoint2D* point);

/**
@brief Sets the length of the offset from the first connection point to the
       start of the first extension line. The connection and extent points will
       not move.
@param[in] dimension The linear dimension object.
@param[in] length    The length the offset to the extension line should be.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOLinearDimensionSetStartOffsetLength(LOLinearDimensionRef dimension, double length);

/**
@brief Sets the length of the offset from the second connection point to the
       start of the second extension line. The connection and extent points
       will not move.
@param[in] dimension The linear dimension object.
@param[in] length    The length the offset to the extension line should be.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOLinearDimensionSetEndOffsetLength(LOLinearDimensionRef dimension, double length);

/**
@brief Connects the linear dimension to the given connection points. If both
       the linear dimension and the connection point's entities are on
       non-shared layers, they must be on the same page.
@param[in] dimension   The linear dimension object.
@param[in] start_point The first connection point object.
@param[in] end_point   The second connection point object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if start_point does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if end_point does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
- \ref SU_ERROR_GENERIC if the linear dimension was unable to connect to the
  connection points
*/
LO_RESULT LOLinearDimensionConnectTo(
    LOLinearDimensionRef dimension, LOConnectionPointRef start_point,
    LOConnectionPointRef end_point);

/**
@brief Disconnects the linear dimension from its connection points. The
       dimension will not be adjusted by disconnecting from its connection
       points.
@param[in] dimension The linear dimension object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if dimension is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if dimension is locked
*/
LO_RESULT LOLinearDimensionDisconnect(LOLinearDimensionRef dimension);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_LINEARDIMENSION_H_
