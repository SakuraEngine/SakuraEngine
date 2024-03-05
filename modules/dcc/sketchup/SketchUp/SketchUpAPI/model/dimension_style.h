// Copyright 2016 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUDimensionStyleRef.
 */
#ifndef SKETCHUP_MODEL_DIMENSION_STYLE_H_
#define SKETCHUP_MODEL_DIMENSION_STYLE_H_

#include <SketchUpAPI/color.h>
#include <SketchUpAPI/model/defs.h>
#include <SketchUpAPI/model/dimension.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUDimensionStyleRef
@brief  A dimension style reference.
@since SketchUp 2017, API 5.0
*/

/**
@brief Retrieves the font of a dimension style object.
@since SketchUp 2017, API 5.0
@param[in]  style The dimension style object.
@param[out] font  The font retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if font is NULL
*/
SU_RESULT SUDimensionStyleGetFont(SUDimensionStyleRef style, SUFontRef* font);

/**
@brief Retrieves whether the dimension style has 3D text.
@since SketchUp 2017, API 5.0
@param[in]  style  The dimension style object.
@param[out] has_3d The flag value retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if has_3d is NULL
*/
SU_RESULT SUDimensionStyleGet3D(SUDimensionStyleRef style, bool* has_3d);

/**
@brief Retrieves a enum value indicating the arrow type specified by the
       dimension style.
@since SketchUp 2017, API 5.0
@param[in]  style The dimension style object.
@param[out] type  The arrow type enum value retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if type is NULL
*/
SU_RESULT SUDimensionStyleGetArrowType(SUDimensionStyleRef style, enum SUArrowType* type);

/**
@brief Retrieves the arrow size specified by the dimension style.
@since SketchUp 2017, API 5.0
@param[in]  style The dimension style object.
@param[out] size  The size retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if size is NULL
*/
SU_RESULT SUDimensionStyleGetArrowSize(SUDimensionStyleRef style, size_t* size);

/**
@brief Retrieves the color specified by the dimension style.
@since SketchUp 2017, API 5.0
@param[in]  style The dimension style object.
@param[out] color The color retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
*/
SU_RESULT SUDimensionStyleGetColor(SUDimensionStyleRef style, SUColor* color);

/**
@brief Retrieves the text color specified by the dimension style.
@since SketchUp 2017, API 5.0
@param[in]  style The dimension style object.
@param[out] color The color retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
*/
SU_RESULT SUDimensionStyleGetTextColor(SUDimensionStyleRef style, SUColor* color);

/**
@brief Retrieves the dimension style's extension line offset. The offset
       specifies the gap between the connection points' locations and the
       dimension leader lines.
@since SketchUp 2017, API 5.0
@param[in]  style  The dimension style object.
@param[out] offset The offset retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if offset is NULL
*/
SU_RESULT SUDimensionStyleGetExtensionLineOffset(SUDimensionStyleRef style, size_t* offset);

/**
@brief Retrieves the dimension style's extension line overshoot. The overshoot
       specifies distance that the dimension leader lines overshoot the point
       where they connect with the arrows.
@since SketchUp 2017, API 5.0
@param[in]  style     The dimension style object.
@param[out] overshoot The overshoot retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if overshoot is NULL
*/
SU_RESULT SUDimensionStyleGetExtensionLineOvershoot(SUDimensionStyleRef style, size_t* overshoot);

/**
@brief Retrieves the line weight specified by the dimension style.
@since SketchUp 2017, API 5.0
@param[in]  style  The dimension style object.
@param[out] weight The weight retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if weight is NULL
*/
SU_RESULT SUDimensionStyleGetLineWeight(SUDimensionStyleRef style, size_t* weight);

/**
@brief Retrieves whether the dimension style specifies non-associative
       dimensions (including edited text) be highlighted in a color which can
       be retrieved with \ref
       SUDimensionStyleGetHighlightNonAssociativeDimensionsColor
@since SketchUp 2017, API 5.0
@param[in]  style     The dimension style object.
@param[out] highlight The flag value retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if highlight is NULL
*/
SU_RESULT SUDimensionStyleGetHighlightNonAssociativeDimensions(
    SUDimensionStyleRef style, bool* highlight);

/**
@brief Retrieves the non-associative dimensions highlight color specified by
       the dimension style.
@since SketchUp 2017, API 5.0
@param[in]  style The dimension style object.
@param[out] color The color retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
*/
SU_RESULT SUDimensionStyleGetHighlightNonAssociativeDimensionsColor(
    SUDimensionStyleRef style, SUColor* color);

/**
@brief Retrieves whether the dimension style specifies that radius/diameter
       prefixes be displayed on radial dimensions.
@since SketchUp 2017, API 5.0
@param[in]  style The dimension style object.
@param[out] show  The flag value retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if show is NULL
*/
SU_RESULT SUDimensionStyleGetShowRadialPrefix(SUDimensionStyleRef style, bool* show);

/**
@brief Retrieves whether the dimension style specifies that out of plane
       dimensions (dimensions which are not parallel to the view plane) be
       hidden.
@since SketchUp 2017, API 5.0
@param[in]  style The dimension style object.
@param[out] hide  The flag value retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if hide is NULL
*/
SU_RESULT SUDimensionStyleGetHideOutOfPlane(SUDimensionStyleRef style, bool* hide);

/**
@brief Retrieves the dimension style's parameter specifying out of plane
       tolerance for hiding dimensions.
@since SketchUp 2017, API 5.0
@param[in]  style     The dimension style object.
@param[out] tolerance The value retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if tolerance is NULL
*/
SU_RESULT SUDimensionStyleGetHideOutOfPlaneValue(SUDimensionStyleRef style, double* tolerance);

/**
@brief Retrieves whether the dimension style specifies that small dimensions be
       hidden.
@since SketchUp 2017, API 5.0
@param[in]  style The dimension style object.
@param[out] hide  The flag value retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if hide is NULL
*/
SU_RESULT SUDimensionStyleGetHideSmall(SUDimensionStyleRef style, bool* hide);

/**
@brief Retrieves the minimum size under which dimensions will be hidden if \ref
       SUDimensionStyleGetHideSmall returns true.
@since SketchUp 2017, API 5.0
@param[in]  style     The dimension style object.
@param[out] tolerance The value retrieved.
@related SUDimensionStyleRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if tolerance is NULL
*/
SU_RESULT SUDimensionStyleGetHideSmallValue(SUDimensionStyleRef style, double* tolerance);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_DIMENSION_STYLE_H_
