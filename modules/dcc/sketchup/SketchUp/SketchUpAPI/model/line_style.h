// Copyright 2018 Trimble Inc. All Rights Reserverd.

/**
 * @file
 * @brief Interfaces for SULineStyleRef.
 */
#ifndef SKETCHUP_SOURCE_SKORE_SKETCHUP_PUBLIC_MODEL_LINESTYLE_H_
#define SKETCHUP_SOURCE_SKORE_SKETCHUP_PUBLIC_MODEL_LINESTYLE_H_

#include <SketchUpAPI/color.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>

#pragma pack(push, 8)
#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SULineStyleRef
@extends SUEntityRef
@brief  References a line style.
*/

/**
@brief Converts from a \ref SULineStyleRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@since SketchUp 2020.1, API 8.1
@param[in] line_style The given line style reference.
@related SULineStyleRef
@return
- The converted \ref SUEntityRef if line_style is a valid line style
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SULineStyleToEntity(SULineStyleRef line_style);

/**
@brief Converts from an \ref SUEntityRef to a \ref SULineStyleRef.
       This is essentially a downcast operation so the given entity must be
       convertible to a \ref SULineStyleRef.
@since SketchUp 2020.1, API 8.1
@param[in] entity_ref  The given entity reference.
@related SULineStyleRef
@return
- The converted \ref SULineStyleRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SULineStyleRef SULineStyleFromEntity(SUEntityRef entity_ref);

/**
@brief Retrieves the name of a line style object.
@since SketchUp 2019, API 7.0
@param[in]  line_style The line style object.
@param[out] name       The name retrieved.
@related SULineStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if line_style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
       SUStringRef object
*/
SU_RESULT SULineStyleGetName(SULineStyleRef line_style, SUStringRef* name);

/**
@brief Retrieves the pixel width of a line style.
@since SketchUp 2019, API 7.0
@param[in]  line_style  The line style object.
@param[out] pixel_width The pixel width retrieved.
@related SULineStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if line_style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if pixel_width is NULL
*/
SU_RESULT SULineStyleGetWidthPixels(SULineStyleRef line_style, double* pixel_width);

/**
@brief Sets the pixel width of a line style. Must be greater than 0.
@note The restrictions on pixel_width were relaxed in SketchUp 2020.2, API 8.2
      from requiring that pixel_width be greater than or equal to one to
      requiring that it be greater than zero.
@since SketchUp 2019, API 7.0
@param[in]  line_style  The line style object.
@param[in]  pixel_width The new pixel width of the line style.
@related SULineStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if line_style is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE if pixel_width is less than or equal to 0.0
- \ref SU_ERROR_INVALID_OPERATION if line_style is a built-in style
*/
SU_RESULT SULineStyleSetWidthPixels(SULineStyleRef line_style, double pixel_width);

/**
@brief Retrieves the length multiplier of a line style. This value is used to
       scale the applied stipple of the line.
@since SketchUp 2019, API 7.0
@param[in]  line_style        The line style object.
@param[out] length_multiplier The length multiplier retrieved.
@related SULineStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if line_style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if length_multiplier is NULL
*/
SU_RESULT SULineStyleGetLengthMultiplier(SULineStyleRef line_style, double* length_multiplier);

/**
@brief Sets the length multiplier of a line style. Must be a non-zero value.
       Default is 1. This value is used to scale the applied stipple pattern
       of the line. Positive values will be applied directly (ie a value of
       2.0 will stretch the stipple pattern by a factor of 2). Negative values
       will be divided by the line width such that the final scale equals
       abs(length_multiplier) / current_line_width.
@note The restrictions on length_multiplier were relaxed in SketchUp 2020.2,
      API 8.2 from requiring that length_multiplier be a positive value to
      requiring that length_multiplier be non-zero.
@since SketchUp 2019, API 7.0
@param[in]  line_style       The line style object.
@param[in]  length_multiplier The new length multiplier of the line style.
@related SULineStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if line_style is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE if length_multiplier is equal to zero
- \ref SU_ERROR_INVALID_OPERATION if line_style is a built-in style
*/
SU_RESULT SULineStyleSetLengthMultiplier(SULineStyleRef line_style, double length_multiplier);

/**
 @brief Retrieves the color value of a line style object.
 @since SketchUp 2020.1, API 8.1
 @param[in]  line_style The line style object.
 @param[out] color      The color value retrieved.
 @related SULineStyleRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_INVALID_INPUT if line_style is not a valid object
 - \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
 - \ref SU_ERROR_NO_DATA if the line style object does not have a color value
 */
SU_RESULT SULineStyleGetColor(SULineStyleRef line_style, SUColor* color);

/**
 @brief Sets the color of a line style object.
 @since SketchUp 2020.1, API 8.1
 @param[in] line_style The line style object.
 @param[in] color      The color value to set the line style color.
 @related SULineStyleRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_INVALID_INPUT if line_style is not a valid object
 - \ref SU_ERROR_NULL_POINTER_INPUT if color is NULL
 - \ref SU_ERROR_INVALID_OPERATION if line_style is a built-in style
 */
SU_RESULT SULineStyleSetColor(SULineStyleRef line_style, const SUColor* color);

/**
 @brief Creates a new line style object by copying an existing one.
 @since SketchUp 2020.1, API 8.1
 @param[in] line_style            The line style object to copy.
 @param[in] name                  The name to apply to the new line style object.
 @param[out] destination          The new line style object that is created.
 @related SULineStyleRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_INVALID_INPUT if line_style is not a valid object
 - \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
 - \ref SU_ERROR_NULL_POINTER_OUTPUT if destination is NULL
 - \ref SU_ERROR_OVERWRITE_VALID if destination already references a valid
     object.
 */
SU_RESULT SULineStyleCreateCopy(
    SULineStyleRef line_style, const char* name, SULineStyleRef* destination);

/**
 @brief Deallocates a line style object.

 The line style object to be deallocated must not be associated with a line
 style manager.
 @since SketchUp 2020.1, API 8.1
 @param[in] line_style            The line style object to release.
 @related SULineStyleRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_NULL_POINTER_INPUT if line_style is NULL
 - \ref SU_ERROR_INVALID_INPUT if line_style is not a valid object
 - \ref SU_ERROR_INVALID_OPERATION if line_style is not a user created style
 */
SU_RESULT SULineStyleRelease(SULineStyleRef* line_style);

/**
 @brief Indicates whether the line style object is user created or not.
 @since SketchUp 2020.1, API 8.1
 @param[in]  line_style      The line style object.
 @param[out] is_user_created The boolean value retrieved.
 @related SULineStyleRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_INVALID_INPUT if line_style is not a valid object
 - \ref SU_ERROR_NULL_POINTER_OUTPUT if is_user_created is NULL
 */
SU_RESULT SULineStyleIsUserCreated(SULineStyleRef line_style, bool* is_user_created);

#ifdef __cplusplus
}
#endif
#pragma pack(pop)

#endif  // SKETCHUP_SOURCE_SKORE_SKETCHUP_PUBLIC_MODEL_LINESTYLE_H_
