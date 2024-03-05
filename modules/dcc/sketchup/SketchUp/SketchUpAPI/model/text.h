// Copyright 2016 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUTextRef.
 */
#ifndef SKETCHUP_MODEL_TEXT_H_
#define SKETCHUP_MODEL_TEXT_H_

#include <SketchUpAPI/color.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/model/defs.h>
#include <SketchUpAPI/model/arrow_type.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUTextRef
@extends SUDrawingElementRef
@brief  A text entity reference.
@since SketchUp 2018, API 6.0
*/

/**
@enum SUTextLeaderType
@brief Indicates the supported leader line types
*/
enum SUTextLeaderType {
  SUTextLeaderType_None = 0,
  SUTextLeaderType_ViewBased,
  SUTextLeaderType_PushPin
};

/**
@brief Converts from an \ref SUTextRef to an \ref SUEntityRef. This is
       essentially an upcast operation.
@since SketchUp 2018, API 6.0
@param[in] text The given text reference.
@related SUTextRef
@return
- The converted \ref SUEntityRef if text is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUTextToEntity(SUTextRef text);

/**
@brief Converts from an SUEntityRef to an \ref SUTextRef. This is
       essentially a downcast operation so the given SUEntityRef must be
       convertible to an \ref SUTextRef.
@since SketchUp 2018, API 6.0
@param[in] entity The given entity reference.
@related SUTextRef
@return
- The converted \ref SUTextRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUTextRef SUTextFromEntity(SUEntityRef entity);

/**
@brief Converts from an \ref SUTextRef to an \ref SUDrawingElementRef.
       This is essentially an upcast operation.
@since SketchUp 2018, API 6.0
@param[in] text The given dimension reference.
@related SUTextRef
@return
- The converted \ref SUDrawingElementRef if text is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDrawingElementRef SUTextToDrawingElement(SUTextRef text);

/**
@brief Converts from an SUDrawingElementRef to an \ref SUTextRef. This is
       essentially a downcast operation so the given SUDrawingElementRef must
       be convertible to an \ref SUTextRef.
@since SketchUp 2018, API 6.0
@param[in] element The given drawing element reference.
@related SUTextRef
@return
- The converted \ref SUTextRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUTextRef SUTextFromDrawingElement(SUDrawingElementRef element);

/**
@brief Creates a text edge object.
       The text object must be subsequently deallocated with \ref SUTextRelease()
       unless the text object is associated with a parent object.
@since SketchUp 2018, API 6.0
@param[out] text The text object.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_OVERWRITE_VALID if text input object reference already
       references an object.
*/
SU_RESULT SUTextCreate(SUTextRef* text);

/**
@brief Releases a text object.
       The text object must have been created with SUTextCreate() and not
       subsequently associated with a parent object (e.g. SUEntitiesAddTexts()).
@since SketchUp 2018, API 6.0
@param[in] text The text object.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text does not reference a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if text is NULL
*/
SU_RESULT SUTextRelease(SUTextRef* text);

/**
@brief Sets the string to the text object
@since SketchUp 2018, API 6.0
@param[in] text   The text object.
@param[in] string The string to set.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_INVALID_INPUT if string is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if string is NULL
*/
SU_RESULT SUTextSetString(SUTextRef text, const char* string);

/**
@brief Retrieves the string from the text object
@since SketchUp 2018, API 6.0
@param[in]  text   The text object.
@param[out] string The string retrieved.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if string is NULL
- \ref SU_ERROR_INVALID_OUTPUT if string does not point to a valid \ref
       SUStringRef object
*/
SU_RESULT SUTextGetString(SUTextRef text, SUStringRef* string);

/**
@brief Sets the font to the text object
@since SketchUp 2018, API 6.0
@param[in] text The text object.
@param[in] font The font to set.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_INVALID_INPUT if font is not a valid object
*/
SU_RESULT SUTextSetFont(SUTextRef text, SUFontRef font);

/**
@brief Retrieves the font from the text object
@since SketchUp 2018, API 6.0
@param[in]  text The text object.
@param[out] font The font retrieved.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if font is NULL
*/
SU_RESULT SUTextGetFont(SUTextRef text, SUFontRef* font);

/**
@brief Sets the leader type to the text object
@since SketchUp 2018, API 6.0
@param[in] text   The text object.
@param[in] leader The leader type to set.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE if leader is not a valid value
*/
SU_RESULT SUTextSetLeaderType(SUTextRef text, enum SUTextLeaderType leader);

/**
@brief Retrieves the leader type from the text object
@since SketchUp 2018, API 6.0
@param[in]  text   The text object.
@param[out] leader The leader type retrieved.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if leader is NULL
*/
SU_RESULT SUTextGetLeaderType(SUTextRef text, enum SUTextLeaderType* leader);

/**
@brief Sets the arrow type to the text object
@since SketchUp 2018, API 6.0
@param[in] text       The text object.
@param[in] arrow_type The arrow type to set.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE if arrow_type is not a valid value
*/
SU_RESULT SUTextSetArrowType(SUTextRef text, enum SUArrowType arrow_type);

/**
@brief Retrieves the arrow type from the text object
@since SketchUp 2018, API 6.0
@param[in]  text       The text object.
@param[out] arrow_type The arrow type retrieved.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if arrow_type is NULL
*/
SU_RESULT SUTextGetArrowType(SUTextRef text, enum SUArrowType* arrow_type);

/**
@brief Sets the connection point of a text object. A text's connection point can
       be set in a few different ways. In the simplest form a connection point
       can be set to an arbitrary point in space by providing a non-null \ref
       SUPoint3D and an invalid \ref SUInstancePathRef. The more complex forms
       to connect the point to a position on an entity in the model by providing
       a valid \ref SUInstancePathRef which refers to an existing model entity.
       In the more complex forms the input SUPoint3D must be non-null for all
       connectable entity types except for vertices and guide points, in which
       case the \ref SUPoint3D argument may be null as it will be ignored. It
       should be noted that when changing a text's connection point the other
       point may need to be adjusted as well. Users may want to verify the other
       connection point after setting this one.
@since SketchUp 2018, API 6.0

@code{.c}
  // Simple Example: Connect to an arbitrary position in space
  SUPoint3D point{ xposition, yposition, zposition};
  SUTextSetPoint(text, &point, SU_INVALID);
@endcode

@code{.c}
  // Vertex Example: Connect to vertex entity
  SUInstancePathRef path = SU_INVALID;
  SUInstancePathCreate(&path);
  SUInstancePathSetLeaf(path, SUVertexToEntity(vertex));
  SUTextSetPoint(text, NULL, path);
@endcode

@code{.c}
  // Edge Example: Connect to nearest point on an instance of an edge entity
  SUPoint3D point{ xposition, yposition, zposition};
  SUInstancePathRef path = SU_INVALID;
  SUInstancePathCreate(&path);
  SUInstancePathPushInstance(path, instance);
  SUInstancePathSetLeaf(path, SUEdgeToEntity(edge));
  SUTextSetPoint(text, &point, path);
@endcode

@param[in] text  The text object.
@param[in] point The point to set.
@param[in] path  The instance path to be set.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if path is invalid and point is NULL
- \ref SU_ERROR_INVALID_ARGUMENT path is valid but refers to an invalid
       instance path
- \ref SU_ERROR_GENERIC if point is NULL and path doesn't have a vertex or
       guide point for its leaf
*/
SU_RESULT SUTextSetPoint(SUTextRef text, const struct SUPoint3D* point, SUInstancePathRef path);

/**
 @brief Retrieves the point associated with the text object. The given instance
        path object either must have been constructed using one of the
        SUInstancePathCreate* functions or it will be generated on the fly if it
        is invalid. It must be released using SUInstancePathRelease() when it
        is no longer needed.
@since SketchUp 2018, API 6.0
@param[in]  text  The text object.
@param[out] point The point retrieved.
@param[out] path  The path retrieved.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point or path are NULL
- \ref SU_ERROR_INVALID_OUTPUT if path is not a valid object
*/
SU_RESULT SUTextGetPoint(SUTextRef text, struct SUPoint3D* point, SUInstancePathRef* path);

/**
@brief Sets the leader vector associated with the text object
@since SketchUp 2018, API 6.0
@param[in] text   The text object.
@param[in] vector The vector to set.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if vector is NULL
*/
SU_RESULT SUTextSetLeaderVector(SUTextRef text, const struct SUVector3D* vector);

/**
@brief Retrieves the leader vector associated with the text object
@since SketchUp 2018, API 6.0
@param[in]  text   The text object.
@param[out] vector The vector retrieved.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
SU_RESULT SUTextGetLeaderVector(SUTextRef text, struct SUVector3D* vector);

/**
@brief Sets the color to the text object
@since SketchUp 2018, API 6.0
@param[in] text  The text object.
@param[in] color The color to set.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if color is NULL
*/
SU_RESULT SUTextSetColor(SUTextRef text, const SUColor* color);

/**
@brief Retrieves the color from the text object
@since SketchUp 2018, API 6.0
@param[in]  text  The text object.
@param[out] color The color retrieved.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
*/
SU_RESULT SUTextGetColor(SUTextRef text, SUColor* color);

/**
@brief Sets the screen position for text with no leader.
@since SketchUp 2019, API 7.0
@param[in] text      The text object.
@param[in] percent_x The x position on screen in a range of 0.0 - 1.0 relative
                     to the screen width.
@param[in] percent_y The y position on screen in a range of 0.0 - 1.0 relative
                     to the screen height.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_NO_DATA if the text has a leader to position with.
- \ref SU_ERROR_OUT_OF_RANGE if a leader exists, or if the percentages are not
       between 0 and 1 inclusive.
*/
SU_RESULT SUTextSetScreenPosition(SUTextRef text, const double percent_x, const double percent_y);

/**
@brief Retrieves the screen location for text with no leader.
@since SketchUp 2019, API 7.0
@param[in]  text      The text object.
@param[out] percent_x The percent of screen width to the text position.
@param[out] percent_y The percent of screen height to the text position.
@related SUTextRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if text is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if either percent is NULL
- \ref SU_ERROR_NO_DATA if text has a leader
*/
SU_RESULT SUTextGetScreenPosition(SUTextRef text, double* percent_x, double* percent_y);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_TEXT_H_
