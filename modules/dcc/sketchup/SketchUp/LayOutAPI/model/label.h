// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_LABEL_H_
#define LAYOUT_MODEL_LABEL_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/geometry/geometry.h>
#include <LayOutAPI/model/defs.h>
#include <LayOutAPI/model/formattedtext.h>

/**
@struct LOLabelRef
@brief References a label entity. A label consists of the label text (formatted
       text entity) and the label leader (path entity). A label may be
       connected to another entity via a \ref LOConnectionPointRef object.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@enum LOLabelLeaderLineType
@brief Defines the different types of leader lines for a label entity.
*/
typedef enum {
  LOLabelLeaderLineType_SingleSegment = 0,  ///< Single segment leader line.
  LOLabelLeaderLineType_TwoSegment,         ///< Two segment leader line.
  LOLabelLeaderLineType_Bezier,             ///< Curved Bezier leader line.
  LOLabelLeaderLineType_Unknown,            ///< Custom leader line.
  LONumLabelLeaderLineTypes
} LOLabelLeaderLineType;

/**
@enum LOLabelTextConnectionType
@brief Defines the different types of label text connections for a label entity.
*/
typedef enum {
  LOLabelTextConnectionType_NoConnection = 0,  ///< Label text behaves disconnected.
  LOLabelTextConnectionType_Automatic,  ///< Automatically choses one of the other types based on
                                        ///< current label geometry.
  LOLabelTextConnectionType_ReverseAutomatic,  ///< Automatically choose the OPPOSITE of the type
                                               ///< indicated by
                                               ///< LOLabelTextConnectionType_Automatic.
  LOLabelTextConnectionType_TopLeft,           ///< Top-left corner of text.
  LOLabelTextConnectionType_CenterLeft,        ///< Vertical center of left side of text.
  LOLabelTextConnectionType_BottomLeft,        ///< Bottom-left corner of text.
  LOLabelTextConnectionType_TopRight,          ///< Top-right corner of text.
  LOLabelTextConnectionType_CenterRight,       ///< Vertical center of right side of text.
  LOLabelTextConnectionType_BottomRight,       ///< Bottom-right corner of text.
  LONumLabelTextConnectionTypes
} LOLabelTextConnectionType;

/**
@brief Creates a new disconnected label object whose text is unbounded.
@param[out] label            The label object.
@param[in]  anchor_point     The anchor point for the label text's position.
@param[in]  anchor_type      Defines which point of the label text is set by
                             anchor_point.
@param[in]  plain_text       The plain text to use for the label text.
@param[in]  leader_line_type The type of leader line this label will have.
@param[in]  target_point     Where the label leader should point to.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if label is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *label already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if anchor_point is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if plain_text is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if target_point is NULL
- \ref SU_ERROR_OUT_OF_RANGE if anchor_type is not a valid value
- \ref SU_ERROR_OUT_OF_RANGE if leader_line_type is not a valid value
- \ref SU_ERROR_GENERIC if plain_text is an empty string
*/
LO_RESULT LOLabelCreateAtPoint(
    LOLabelRef* label, const LOPoint2D* anchor_point, LOFormattedTextAnchorType anchor_type,
    const char* plain_text, LOLabelLeaderLineType leader_line_type, const LOPoint2D* target_point);

/**
@brief Creates a new disconnected label object whose text is bounded.
@param[out] label            The label object.
@param[in]  bounds           The label text bounds.
@param[in]  plain_text       The plain text to use as the label text.
@param[in]  leader_line_type The type of leader line this label will have.
@param[in]  target_point     Where the label leader should point to.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if label is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *label already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if bounds is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if plain_text is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if target_point is NULL
- \ref SU_ERROR_GENERIC if plain_text is an empty string
- \ref SU_ERROR_OUT_OF_RANGE if bounds is zero sized
- \ref SU_ERROR_OUT_OF_RANGE if leader_line_type is not a valid value
*/
LO_RESULT LOLabelCreateWithBounds(
    LOLabelRef* label, const LOAxisAlignedRect2D* bounds, const char* plain_text,
    LOLabelLeaderLineType leader_line_type, const LOPoint2D* target_point);

/**
@brief Adds a reference to a label object.
@param[in] label The label object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if label does not refer to a valid object
*/
LO_RESULT LOLabelAddReference(LOLabelRef label);

/**
@brief Releases a label object. The object will be invalidated if
       releasing the last reference.
@param[in] label The label object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if label is NULL
- \ref SU_ERROR_INVALID_INPUT if *label does not refer to a valid object
*/
LO_RESULT LOLabelRelease(LOLabelRef* label);

/**
@brief Converts from a \ref LOEntityRef to a \ref LOLabelRef.
       This is essentially a downcast operation so the given \ref LOEntityRef
       must be convertible to a \ref LOLabelRef.
@param[in] entity The entity object.
@return
- The converted \ref LOLabelRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
LO_EXPORT LOLabelRef LOLabelFromEntity(LOEntityRef entity);

/**
@brief Converts from a \ref LOLabelRef to a \ref LOEntityRef.
       This is essentially an upcast operation.
@param[in] label The label object.
@return
- The converted \ref LOEntityRef if label is a valid object
- If not, the returned reference will be invalid
*/
LO_EXPORT LOEntityRef LOLabelToEntity(LOLabelRef label);

/**
@brief Creates the entities that represent the label in its exploded form and
       adds them to a \ref LOEntityListRef. It is NOT necessary to explicitly
       release these entities, since \ref LOEntityListRef itself adds a
       reference to the entities and will release them when they are removed
       from the list or when the list is released.
@param[in] label             The label object.
@param[in] entity_list       The entity list object to add the new entities to.
@param[in] page_for_autotext The page that is currently being imported, exported,
                             or displayed. This must be a valid object if
                             auto-text tags should be substituted with their
                             display representations in the text that is
                             returned. Otherwise, this object may be invalid.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if label does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if entity_list does not refer to a valid object
- \ref SU_ERROR_GENERIC if page_for_autotext is a valid object and does not
  belong to the same document as label.
*/
LO_RESULT LOLabelGetExplodedEntities(
    LOLabelRef label, LOEntityListRef entity_list, LOPageRef page_for_autotext);

/**
@brief Connects the label to the given connection point. The leader line will
       be adjusted to point at the connection point. The label must be in the
       same document as the connection point. If both the label and the
       connection point's entity are on non-shared layers, they must be on the
       same page.
@param[in] label            The label object.
@param[in] connection_point The connection point object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if label does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if connection_point does not refer to a valid
  object
- \ref SU_ERROR_LAYER_LOCKED if label is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if label is locked
- \ref SU_ERROR_GENERIC if the label was unable to connect to the connection
  point
*/
LO_RESULT LOLabelConnectTo(LOLabelRef label, LOConnectionPointRef connection_point);

/**
@brief Disconnects the label from its connection point. The leader line will
       not be adjusted by disconnecting from a connection point.
@param[in] label The label object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if label does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if label is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if label is locked
*/
LO_RESULT LOLabelDisconnect(LOLabelRef label);

/**
@brief Creates a copy of the label text's \ref LOFormattedTextRef object.
@param[in]  label The label object.
@param[out] text  A copy of the formatted text object representing the label
                  text.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if label does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if text is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *text already refers to a valid object
*/
LO_RESULT LOLabelCreateLabelTextCopy(LOLabelRef label, LOFormattedTextRef* text);

/**
@brief Creates a copy of the label text's \ref LOFormattedTextRef object. This
       copy will have all auto-text tags substituted with the display text. A
       valid page object must be provided for page name/number auto-text to
       be correctly substituted.
@param[in]  label The label object.
@param[in]  page  The page object.
@param[out] text  A copy of the formatted text object representing the label
                  display text.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if label does not refer to a valid object
- \ref SU_ERROR_GENERIC if page and label don't belong to the same document
- \ref SU_ERROR_NULL_POINTER_OUTPUT if text is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *text already refers to a valid object
*/
LO_RESULT LOLabelCreateLabelDisplayTextCopy(
    LOLabelRef label, LOPageRef page, LOFormattedTextRef* text);

/**
@brief Sets the text of a label from a \ref LOFormattedTextRef object. The
       leader line will be adjusted to connect to the new label text if its
       bounds are different.
@param[in] label The label object.
@param[in] text  The formatted text object representing the label text.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if label does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if label is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if label is locked
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
*/
LO_RESULT LOLabelSetLabelText(LOLabelRef label, LOFormattedTextRef text);

/**
@brief Creates a copy of the label leader's \ref LOPathRef object.
@param[in]  label The label object.
@param[out] path  A copy of the path object representing the label leader.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if label does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if path is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *path already refers to a valid object
*/
LO_RESULT LOLabelCreateLeaderLineCopy(LOLabelRef label, LOPathRef* path);

/**
@brief Sets the leader line of a label from a \ref LOPathRef object. The label
       text position will change to remain connected to the end point of the
       leader line. The leader line type may change based on number and types
       of points in the path.
@param[in] label The label object.
@param[in] path  The path object representing the label leader.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if label does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if path does not refer to a valid object
- \ref SU_ERROR_GENERIC if path contains less than two points
- \ref SU_ERROR_LAYER_LOCKED if label is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if label is locked
*/
LO_RESULT LOLabelSetLeaderLine(LOLabelRef label, LOPathRef path);

/**
@brief Gets the type of leader line the label is using. If the leader line has
       been customized, the type may be unknown.
@param[in]  label            The label object.
@param[out] leader_line_type The leader line type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if label does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if leader_line_type is NULL
*/
LO_RESULT LOLabelGetLeaderLineType(LOLabelRef label, LOLabelLeaderLineType* leader_line_type);

/**
@brief Sets the type of leader line the label is using. New control points will
       be generated if changing to a two segment or bezier leader line type.
@param[in] label            The label object.
@param[in] leader_line_type The leader line type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if label does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if leader_line_type is
  LOLabelLeaderLineType_Unknown or not a valid leader line type
- \ref SU_ERROR_LAYER_LOCKED if label is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if label is locked
*/
LO_RESULT LOLabelSetLeaderLineType(LOLabelRef label, LOLabelLeaderLineType leader_line_type);

/**
@brief Gets the type of connection between the label text and leader line.
@param[in]  label                The label object.
@param[out] text_connection_type The text connection type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if label does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if text_connection_type is NULL
*/
LO_RESULT LOLabelGetTextConnectionType(
    LOLabelRef label, LOLabelTextConnectionType* text_connection_type);

/**
@brief Sets the type of connection between the label text and leader line. The
       text position will change to account for a change in text connection
       type.
@param[in] label                The label object.
@param[in] text_connection_type The text connection type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if label does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if label is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED
- \ref SU_ERROR_OUT_OF_RANGE if text_connection_type is not a valid value
*/
LO_RESULT LOLabelSetTextConnectionType(
    LOLabelRef label, LOLabelTextConnectionType text_connection_type);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_LABEL_H_
