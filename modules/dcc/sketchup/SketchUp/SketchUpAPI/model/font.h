// Copyright 2016 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUFontRef.
 */
#ifndef SKETCHUP_MODEL_FONT_H_
#define SKETCHUP_MODEL_FONT_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUFontRef
@extends SUEntityRef
@brief  A font entity reference.
@since SketchUp 2017, API 5.0
*/

/**
@brief Retrieves the face name of a font object.
@since SketchUp 2017, API 5.0
@param[in]  font The font object.
@param[out] name  The name retrieved.
@related SUFontRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if font is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
SUStringRef object
*/
SU_RESULT SUFontGetFaceName(SUFontRef font, SUStringRef* name);

/**
@brief Retrieves a font's point size.
@since SketchUp 2017, API 5.0
@param[in]  font The font object.
@param[out] size The returned font size.
@related SUFontRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if font is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if size is NULL
*/
SU_RESULT SUFontGetPointSize(SUFontRef font, size_t* size);

/**
@brief Retrieves a boolean indicating whether the font is bold.
@since SketchUp 2017, API 5.0
@param[in]  font    The font object.
@param[out] is_bold The boolean retrieved.
@related SUFontRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if font is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_bold is NULL
*/
SU_RESULT SUFontGetBold(SUFontRef font, bool* is_bold);

/**
@brief Retrieves a boolean indicating whether the font is italic.
@since SketchUp 2017, API 5.0
@param[in]  font      The font object.
@param[out] is_italic The boolean retrieved.
@related SUFontRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if font is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_italic is NULL
*/
SU_RESULT SUFontGetItalic(SUFontRef font, bool* is_italic);

/**
@brief Retrieves a boolean indicating whether the size of the font is defined
       as a height in world space. A false value indicates that the font size
       is defined in points (i.e. in screen space).
@since SketchUp 2017, API 5.0
@param[in]  font           The font object.
@param[out] use_world_size The boolean retrieved.
@related SUFontRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if font is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_italic is NULL
*/
SU_RESULT SUFontGetUseWorldSize(SUFontRef font, bool* use_world_size);

/**
@brief Retrieves the height of the font in inches when the font size is defined
       as a height in world space. That is, when \ref SUFontGetUseWorldSize()
       returns true.
@since SketchUp 2017, API 5.0
@param[in]  font       The font object.
@param[out] world_size The returned world size factor.
@related SUFontRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if font is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if world_size is NULL
*/
SU_RESULT SUFontGetWorldSize(SUFontRef font, double* world_size);

/**
@brief Converts from an \ref SUFontRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@since SketchUp 2019, API 7.0
@param[in] font The given font reference.
@related SUFontRef
@return
- The converted SUEntityRef if font is a valid font.
- If not, the returned reference will be invalid.
*/
SU_EXPORT SUEntityRef SUFontToEntity(SUFontRef font);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUFontRef.
       This is essentially a downcast operation so the given entity must be
       convertible to an \ref SUFontRef.
@since SketchUp 2019, API 7.0
@param[in] entity The given entity reference.
@related SUFontRef
@return
- The converted SUFontRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUFontRef SUFontFromEntity(SUEntityRef entity);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_FONT_H_
