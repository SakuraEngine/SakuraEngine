// Copyright 2018 Trimble Inc. All Rights Reserverd.

/**
 * @file
 * @brief Interfaces for SULineStylesRef.
 */
#ifndef SKETCHUP_SOURCE_SKORE_SKETCHUP_PUBLIC_MODEL_LINESTYLES_H_
#define SKETCHUP_SOURCE_SKORE_SKETCHUP_PUBLIC_MODEL_LINESTYLES_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>

#pragma pack(push, 8)
#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SULineStylesRef
@brief  Provides access to the different line style objects in the model.
*/

/**
@brief  Gets the number of line styles.
@since SketchUp 2019, API 7.0
@param[in]  line_styles The line_styles manager object.
@param[out] count       The number of line styles available.
@related SULineStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if line_style_manager is not valid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SULineStylesGetNumLineStyles(SULineStylesRef line_styles, size_t* count);

/**
@brief  Retrieves line styles associated with the line styles manager.
@since SketchUp 2019, API 7.0
@param[in] line_styles                 The line_styles manager object.
@param[in] len                         The number of line style names
    to retrieve.
@param[out] line_styles_provider_names The line_style names retrieved.
@param[out] count                      The number of line style names
retrieved.
@related SULineStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if line_style_manager is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if line_styles_providers or count is NULL
- \ref SU_ERROR_INVALID_OUTPUT if any of the strings in line_styles_provider_names
are invalid
*/
SU_RESULT SULineStylesGetLineStyleNames(
    SULineStylesRef line_styles, size_t len, SUStringRef line_styles_provider_names[],
    size_t* count);

/**
@brief  Retrieves the line styles provider given a name.
@since SketchUp 2019, API 7.0
@param[in] line_styles           The line_styles manager object.
@param[in] name                  The name of the line style object to
     get. Assumed to be UTF-8 encoded.
@param[out] line_style           The line style object retrieved.
@related SULineStylesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if line_styles is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if line_style is NULL
- \ref SU_ERROR_NO_DATA if name does not match the name of any existing style.
*/
SU_RESULT SULineStylesGetLineStyleByName(
    SULineStylesRef line_styles, const char* name, SULineStyleRef* line_style);


#ifdef __cplusplus
}
#endif
#pragma pack(pop)

#endif  // SKETCHUP_SOURCE_SKORE_SKETCHUP_PUBLIC_MODEL_LINESTYLES_H_
