// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_FORMATTED_TEXT_H_
#define LAYOUT_MODEL_FORMATTED_TEXT_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/geometry/geometry.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOFormattedTextRef
@brief References a formatted text entity.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@enum LOFormattedTextGrowMode
@brief Defines the different grow modes that are available for formatted text
       entities.
*/
typedef enum {
  LOFormattedTextGrowMode_Unbounded =
      0,  ///< The text bounds will automatically adjust itself to the size of the current text, and
          ///< lines will not wrap.
  LOFormattedTextGrowMode_Bounded,  ///< The text bounds is set explicitly. Lines that go beyond the
                                    ///< right side of the bounds will wrap, and may not be visible
                                    ///< if wrapping causes the text to go beyond the bottom of the
                                    ///< bounds.
  LONumFormattedTextGrowModes
} LOFormattedTextGrowMode;

/**
@enum LOFormattedTextAnchorType
@brief Defines the different anchor types for a formatted text entity. The
       anchor type determines the point on a text entity's bounds that an
       anchor point refers to.
*/
typedef enum {
  LOFormattedTextAnchorType_TopLeft,       ///< Top-left corner of the text.
  LOFormattedTextAnchorType_CenterLeft,    ///< Center-left side of the text.
  LOFormattedTextAnchorType_BottomLeft,    ///< Bottom-left corner of the text.
  LOFormattedTextAnchorType_TopRight,      ///< Top-right corner of the text.
  LOFormattedTextAnchorType_CenterRight,   ///< Center-right side of the text.
  LOFormattedTextAnchorType_BottomRight,   ///< Bottom-right corner of the text.
  LOFormattedTextAnchorType_TopCenter,     ///< Top-centered text.
  LOFormattedTextAnchorType_CenterCenter,  ///< Centered text.
  LOFormattedTextAnchorType_BottomCenter,  ///< Bottom-centered text.
  LONumFormattedTextAnchorTypes
} LOFormattedTextAnchorType;

/**
@brief Creates a new unbounded text object at the given position.
@param[out] text         The formatted text object.
@param[in]  anchor_point The anchor point for the text object's position.
@param[in]  anchor_type  Defines which point of the text object is set by
                         anchor_point.
@param[in]  plain_text   The plain text to use for the formatted text object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if text is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *text already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if anchor_point is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if plain_text is NULL
- \ref SU_ERROR_OUT_OF_RANGE if anchor_type is not a valid value
- \ref SU_ERROR_GENERIC if plain_text is an empty string
*/
LO_RESULT LOFormattedTextCreateAtPoint(
    LOFormattedTextRef* text, const LOPoint2D* anchor_point, LOFormattedTextAnchorType anchor_type,
    const char* plain_text);

/**
@brief Creates a new bounded text object with the given bounds.
@param[out] text       The formatted text object.
@param[in]  bounds     The text bounds.
@param[in]  plain_text The plain text to use for the formatted text object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if text is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *text already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if bounds is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if plain_text is NULL
- \ref SU_ERROR_GENERIC if plain_text is an empty string
- \ref SU_ERROR_OUT_OF_RANGE if bounds is zero sized
*/
LO_RESULT LOFormattedTextCreateWithBounds(
    LOFormattedTextRef* text, const LOAxisAlignedRect2D* bounds, const char* plain_text);

/**
@brief Creates a new unbounded text object at the given position whose contents
       are linked to the plain text or RTF file at the given path.
@param[out] text         The formatted text object.
@param[in]  anchor_point The anchor point for the text object's position.
@param[in]  anchor_type  Defines which point of the text object is set by
                         anchor_point.
@param[in]  path         The path to the plain text or RTF file.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if text is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *text already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if path is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if anchor_point is NULL
- \ref SU_ERROR_OUT_OF_RANGE if anchor_type is not a valid value
- \ref SU_ERROR_GENERIC if the file referred to by path is empty
- \ref SU_ERROR_NO_DATA if the text file could not be found
*/
LO_RESULT LOFormattedTextCreateAtPointFromFile(
    LOFormattedTextRef* text, const LOPoint2D* anchor_point, LOFormattedTextAnchorType anchor_type,
    const char* path);

/**
@brief Creates a new bounded text object with the given bounds whose
       contents are linked to the plain text or RTF file at the given path.
@param[out] text       The formatted text object.
@param[in]  path       The path to the file containing the text to use.
@param[in]  bounds     The text bounds.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if text is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *text already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if path is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if bounds is NULL
- \ref SU_ERROR_OUT_OF_RANGE if bounds is zero sized
- \ref SU_ERROR_GENERIC if the text file is empty
- \ref SU_ERROR_NO_DATA if the text file could not be found
*/
LO_RESULT LOFormattedTextCreateWithBoundsFromFile(
    LOFormattedTextRef* text, const char* path, const LOAxisAlignedRect2D* bounds);

/**
@brief Adds a reference to a formatted text object.
@param[in] text The formatted text object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
*/
LO_RESULT LOFormattedTextAddReference(LOFormattedTextRef text);

/*
@brief Releases a formatted text object. The object will be invalidated if
       releasing the last reference.
@param[in] text The formatted text object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if text is NULL
- \ref SU_ERROR_INVALID_INPUT if *text does not refer to a valid object
*/
LO_RESULT LOFormattedTextRelease(LOFormattedTextRef* text);

/**
@brief Converts from a \ref LOEntityRef to a \ref LOFormattedTextRef.
       This is essentially a downcast operation so the given \ref LOEntityRef
       must be convertible to a \ref LOFormattedTextRef.
@param[in] entity The entity object.
@return
- The converted \ref LOFormattedTextRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
LO_EXPORT LOFormattedTextRef LOFormattedTextFromEntity(LOEntityRef entity);

/**
@brief Converts from a \ref LOFormattedTextRef to a \ref LOEntityRef.
       This is essentially an upcast operation.
@param[in] text The formatted text object.
@return
- The converted \ref LOEntityRef if model is a valid object
- If not, the returned reference will be invalid
*/
LO_EXPORT LOEntityRef LOFormattedTextToEntity(LOFormattedTextRef text);

/**
@brief Gets the raw RTF representation of a formatted text object.
       NOTE: Passing an invalid page will prevent an auto-text tag from being
       substituted with its display representation.
@param[in]  text              The formatted text object.
@param[out] rtf_text          The RTF text string.
@param[in]  page_for_autotext The page that is currently being imported, exported,
                              or displayed. This must be a valid object if
                              auto-text tags should be substituted with their
                              display representations in the string that is
                              returned. Otherwise, this object may be invalid.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if rtf_text is NULL
- \ref SU_ERROR_INVALID_OUTPUT if *rtf_text does not refer to a valid object
- \ref SU_ERROR_GENERIC if page_for_autotext refers to a page in a different
  document from rtf_text.
*/
LO_RESULT LOFormattedTextGetRTF(
    LOFormattedTextRef text, SUStringRef* rtf_text, LOPageRef page_for_autotext);

/**
@brief Sets the raw RTF representation of a formatted text object.
@param[in] text     The formatted text object.
@param[in] rtf_text The RTF text string.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if text is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if text is locked
- \ref SU_ERROR_NULL_POINTER_INPUT if rtf_text is NULL
- \ref SU_ERROR_GENERIC if rtf_text is an empty string
*/
LO_RESULT LOFormattedTextSetRTF(LOFormattedTextRef text, const char* rtf_text);

/**
@brief Gets the display text representation of a formatted text object.
       NOTE: Passing an invalid page will prevent an auto-text tag from being
       substituted with its display representation.
@since LayOut 2018, API 3.0
@param[in]  text              The formatted text object.
@param[out] display_text      The plain text representation.
@param[in]  page_for_autotext The page that is currently being imported, exported,
                              or displayed. This must be a valid object if
                              auto-text tags should be substituted with their
                              display representations in the string that is
                              returned. Otherwise, this object may be invalid.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if display_text is NULL
- \ref SU_ERROR_INVALID_OUTPUT if *display_text does not refer to a valid object
- \ref SU_ERROR_GENERIC if page_for_autotext refers to a page in a different
  document from display_text.
*/
LO_RESULT LOFormattedTextGetDisplayText(
    LOFormattedTextRef text, SUStringRef* display_text, LOPageRef page_for_autotext);

/**
@brief Gets the length of a formatted text object's plain text representation.
@param[in]  text   The formatted text object.
@param[out] length The length of the plain text representation.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if length is NULL
*/
LO_RESULT LOFormattedTextGetPlainTextLength(LOFormattedTextRef text, size_t* length);

/**
@brief Gets the plain text representation of a formatted text object.
@param[in]  text       The formatted text object.
@param[out] plain_text The plain text representation.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if plain_text is NULL
- \ref SU_ERROR_INVALID_OUTPUT if *plain_text does not refer to a valid object
*/
LO_RESULT LOFormattedTextGetPlainText(LOFormattedTextRef text, SUStringRef* plain_text);

/**
@brief Sets the plain text representation of a formatted text object.
@param[in] text       The formatted text object.
@param[in] plain_text The plain text representation.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if text is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if text is locked
- \ref SU_ERROR_NULL_POINTER_INPUT if plain_text is NULL
- \ref SU_ERROR_GENERIC if plain_text is an empty string
*/
LO_RESULT LOFormattedTextSetPlainText(LOFormattedTextRef text, const char* plain_text);

/**
@brief Sets the plain text representation of a formatted text object and apply
       the given style to the text.
@param[in] text       The formatted text object.
@param[in] plain_text The plain text representation.
@param[in] style      The style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if text is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if text is locked
- \ref SU_ERROR_NULL_POINTER_INPUT if plain_text is NULL
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_GENERIC if plain_text is an empty string
*/
LO_RESULT LOFormattedTextSetTextWithStyle(
    LOFormattedTextRef text, const char* plain_text, LOStyleRef style);

/**
@brief Appends the specified plain text to a formatted text object and apply
       the given style to the appended text. NOTE: this method does not support
       more than two different style runs in a single text string.
@param[in] text       The formatted text object.
@param[in] plain_text The plain text to append.
@param[in] style      The style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if text is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if text is locked
- \ref SU_ERROR_NULL_POINTER_INPUT if plain_text is NULL
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
*/
LO_RESULT LOFormattedTextAppendTextWithStyle(
    LOFormattedTextRef text, const char* plain_text, LOStyleRef style);

/**
@brief Gets the style of a formatted text object at the specified plain text
       character index.
@param[in]  text  The formatted text object.
@param[in]  index The index of the character position to get the style of. Must
                  be greater than or equal to 0, and less than the length
                  returned by \ref LOFormattedTextGetPlainTextLength.
@param[out] style The style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_INVALID_OUTPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is out of range of the plain text length
*/
LO_RESULT LOFormattedTextGetStyleAtCharacter(
    LOFormattedTextRef text, size_t index, LOStyleRef style);

/**
@brief Gets the style starting at the specified plain text character index, and
       running through length characters.
@param[in]  text  The formatted text object.
@param[in]  index The index of the character position to get the style of. Must
                  be greater than or equal to 0, and less than the length
                  returned by \ref LOFormattedTextGetPlainTextLength.
@param[in] length The number of characters to get the style of.
                  index + length must be less than or equal to the
                  length returned by \ref LOFormattedTextGetPlainTextLength.
@param[out] style The style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_INVALID_OUTPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is out of range of the plain
  text length
- \ref SU_ERROR_OUT_OF_RANGE if the range specified by range_begin and
  range_length is invalid for this text.
*/
LO_RESULT LOFormattedTextGetStyleRunAtCharacter(
    LOFormattedTextRef text, size_t index, size_t length, LOStyleRef style);

/**
@brief Sets the style for a range of characters.
@param[in] text         The formatted text object.
@param[in] style        The style object.
@param[in] range_begin  The index of the first character to change the style
                        of. Must be greater than or equal to 0, and less than
                        the length returned by
                        \ref LOFormattedTextGetPlainTextLength.
@param[in] range_length The number of characters to apply the style to.
                        range_begin + range_length must be less than or equal
                        to the length returned by \ref
                        LOFormattedTextGetPlainTextLength.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if text is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if text is locked
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if the range specified by range_begin and
  range_length is invalid for this text.
*/
LO_RESULT LOFormattedTextSetStyleForRange(
    LOFormattedTextRef text, LOStyleRef style, size_t range_begin, size_t range_length);

/**
@brief Gets the mode for how the text box sizes itself.
@param[in] text The formatted text object.
@param[in] mode The mode value.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if mode is NULL
*/
LO_RESULT LOFormattedTextGetGrowMode(LOFormattedTextRef text, LOFormattedTextGrowMode* mode);

/**
@brief Sets the mode for how the text box sizes itself.
@param[in] text The formatted text object.
@param[in] mode The mode value.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if text is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if text is locked
- \ref SU_ERROR_OUT_OF_RANGE if the value of mode is invalid
*/
LO_RESULT LOFormattedTextSetGrowMode(LOFormattedTextRef text, LOFormattedTextGrowMode mode);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus
#endif  // LAYOUT_MODEL_FORMATTED_TEXT_H_
