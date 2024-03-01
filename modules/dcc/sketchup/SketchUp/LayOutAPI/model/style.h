// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_STYLE_H_
#define LAYOUT_MODEL_STYLE_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOStyleRef
@brief References a collection of style attributes that determine the
       visual appearance of entities. Style attributes are those attributes
       which the user can manipulate in LayOut's inspector windows. For
       example, shape style attributes that define stroke and fill, or text
       style attributes that define the font for formatted text. The document
       maintains a default style for various types of entities, and it is
       possible to apply the style of one entity to another. \ref LOStyleRef
       objects are transient and do not belong to a document.
*/

/**
@enum LOStrokeJoinStyle
@brief Defines how to draw the connection between two segments of a path's
       stroke.
*/
typedef enum {
  LOStrokeJoinStyle_Miter = 0,  ///< The corner between two segments will be pointed.
  LOStrokeJoinStyle_Round,      ///< The corner between two segments will be rounded.
  LOStrokeJoinStyle_Bevel,      ///< The corner between two segments will be flat.
  LONumStrokeJoinStyles
} LOStrokeJoinStyle;

/**
@enum LOStrokeCapStyle
@brief Defines how to draw the end-caps of a path's stroke.
*/
typedef enum {
  LOStrokeCapStyle_Flat = 0,  ///< The end caps will be flat.
  LOStrokeCapStyle_Round,     ///< The end caps will be round.
  LOStrokeCapStyle_Square,    ///< The end caps will be square.
  LONumStrokeCapStyles
} LOStrokeCapStyle;

/**
@enum LOStrokePattern
@brief Defines the stippling pattern of a path's stroke.
*/
typedef enum {
  LOStrokePattern_Solid = 0,          ///< Solid pattern.
  LOStrokePattern_Dash,               ///< Dashed pattern.
  LOStrokePattern_Dot,                ///< Dotted pattern.
  LOStrokePattern_DashDot,            ///< Repeating dash-dot pattern.
  LOStrokePattern_DashDotDot,         ///< Repeating dash-dot-dot pattern.
  LOStrokePattern_DashSpace,          ///< Deprecated in 2020.2. Use LOStrokePattern_Dash instead.
  LOStrokePattern_DashDotDotDot,      ///< Repeating dash-dot-dot-dot pattern.
  LOStrokePattern_DashDashDot,        ///< Repeating dash-dash-dot pattern.
  LOStrokePattern_DashDashDotDot,     ///< Repeating dash-dash-dot-dot pattern.
  LOStrokePattern_DashDashDotDotDot,  ///< Repeating dash-dash-dot-dot-dot pattern.
  LOStrokePattern_Center,             ///< Repeating center line type pattern.
  LOStrokePattern_Phantom,            ///< Repeating phantom line type pattern.
  LOStrokePattern_ShortDash,          ///< Repeating short dash pattern.
  LONumStrokePatterns
} LOStrokePattern;

/**
@enum LOTextUnderline
@brief Defines underline styles for formatted text.
*/
typedef enum {
  LOTextUnderline_None = 0,  ///< No underline.
  LOTextUnderline_Single,    ///< Single underline.
  LOTextUnderline_Double,    ///< Double underline.
  LONumTextUnderlines
} LOTextUnderline;

/**
@enum LOTextElevation
@brief Defines super/subscript options for formatted text.
*/
typedef enum {
  LOTextElevation_Normal = 0,   ///< Normal text.
  LOTextElevation_Superscript,  ///< Superscript text.
  LOTextElevation_Subscript,    ///< Subscript text.
  LONumTextElevations
} LOTextElevation;

/**
@enum LOTextAlignment
@brief Defines left/right/center text alignment for formatted text.
*/
typedef enum {
  LOTextAlignment_Left = 0,  ///< Left-aligned text.
  LOTextAlignment_Right,     ///< Right-aligned text.
  LOTextAlignment_Center,    ///< Center-aligned text.
  LONumTextAlignments
} LOTextAlignment;

/**
@enum LOTextAnchor
@brief Defines top/center/bottom text anchoring for formatted text.
*/
typedef enum {
  LOTextAnchor_Top = 0,  ///< Top-anchored text.
  LOTextAnchor_Center,   ///< Center-anchored text.
  LOTextAnchor_Bottom,   ///< Bottom-anchored text.
  LONumTextAnchors
} LOTextAnchor;

/**
@enum LODimensionRotationAlignment
@brief Represents how the dimension text is oriented on the page.
*/
typedef enum {
  LODimensionRotationAlignment_Horizontal = 0,  ///< Align the text horizontally.
  LODimensionRotationAlignment_Vertical,        ///< Align the text vertically.
  LODimensionRotationAlignment_Align,           ///< Align the text parallel to the dimension line.
  LODimensionRotationAlignment_Perpendicular,   ///< Align the text perpendicular to the dimension
                                                ///< line.
  LONumDimensionRotationAlignments
} LODimensionRotationAlignment;

/**
@enum LODimensionVerticalAlignment
@brief Represents how the dimension text is positioned. The position is
       relative to the dimension line. \ref LODimensionVerticalAlignment_Offset
       indicates that the text box position is based on a custom offset.
*/
typedef enum {
  LODimensionVerticalAlignment_Above = 0,  ///< Position the text above the dimension line.
  LODimensionVerticalAlignment_Center,     ///< Position the text on top of the dimension line.
  LODimensionVerticalAlignment_Below,      ///< Position the text under the dimension line.
  LODimensionVerticalAlignment_Offset,     ///< Position the text in an arbitrary location.
  LONumDimensionVerticalAlignments
} LODimensionVerticalAlignment;

/**
@enum LODimensionUnits
@brief Defines the different units formats that are available for linear and
       angular dimensions.
*/
typedef enum {
  // Linear dimension units
  LODimensionUnits_FractionalInches = 0,  ///< 60"
  LODimensionUnits_ArchitecturalInches,   ///< 6-1/2"
  LODimensionUnits_EngineeringFeet,       ///< 7.250'
  LODimensionUnits_DecimalInches,         ///< 6.5"
  LODimensionUnits_DecimalFeet,           ///< 0.54167'
  LODimensionUnits_DecimalMillimeters,    ///< 165.1 mm
  LODimensionUnits_DecimalCentimeters,    ///< 16.51 cm
  LODimensionUnits_DecimalMeters,         ///< 1.651 m
  LODimensionUnits_DecimalPoints,         ///< 468 pt

  // Angular dimension units
  LODimensionUnits_Degrees,  ///< 21°
  LODimensionUnits_Radians,  ///< 0.36 r

  LONumDimensionUnits
} LODimensionUnits;



/**
@enum LOArrowType
@brief Defines the arrowhead types available for a path entity.
*/
typedef enum {
  LOArrowType_None = 0,              ///< No arrowhead.
  LOArrowType_FilledTriangle,        ///< Filled triangle arrowhead.
  LOArrowType_OpenTriangle,          ///< Non-filled triangle arrowhead.
  LOArrowType_FilledSkinnyTriangle,  ///< Filled skinny triangle arrowhead.
  LOArrowType_OpenSkinnyTriangle,    ///< Non-filled skinny triangle arrowhead.
  LOArrowType_OpenArrow90,           ///< 90 degree open-sided arrowhead.
  LOArrowType_OpenArrow120,          ///< 120 degree open-sided arrowhead.
  LOArrowType_FilledCircle,          ///< Filled circle arrowhead.
  LOArrowType_OpenCircle,            ///< Non-filled circle arrowhead.
  LOArrowType_FilledSquare,          ///< Filled square arrowhead.
  LOArrowType_OpenSquare,            ///< Non-filled square arrowhead.
  LOArrowType_FilledDiamond,         ///< Filled diamond arrowhead.
  LOArrowType_OpenDiamond,           ///< Non-filled diamond arrowhead.
  LOArrowType_Star,                  ///< Star-shaped arrowhead.
  LOArrowType_T,                     ///< T-shaped arrowhead.
  LOArrowType_SlashRight,            ///< Right slash arrowhead.
  LOArrowType_SlashLeft,             ///< Left slash arrowhead.
  LOArrowType_Underrun,              ///< Underrun arrowhead.
  LOArrowType_Overrun,               ///< Overrun arrowhead.
  LONumArrowTypes
} LOArrowType;

/**
@enum LOSubEntityType
@brief Defines the types of sub entity that may have style attributes applied
       directly to them.
*/
typedef enum {
  LOSubEntityType_LabelLeaderLine = 0,          ///< A label's leader line object.
  LOSubEntityType_LabelText,                    ///< A label's text object.
  LOSubEntityType_DimensionStartExtensionLine,  ///< A dimension's start extension line object.
  LOSubEntityType_DimensionEndExtensionLine,    ///< A dimension's end extension line object.
  LOSubEntityType_DimensionDimensionLine,       ///< A dimension's dimension line object.
  LOSubEntityType_DimensionLeaderLine,          ///< A dimension's leader line object.
  LOSubEntityType_DimensionText,                ///< A dimension's text object.
  LONumSubEntityTypes
} LOSubEntityType;

#include "entity.h"
#include <SketchUpAPI/color.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Creates a new empty style object.
@param[out] style The style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if style is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *style already refers to a valid object
*/
LO_RESULT LOStyleCreate(LOStyleRef* style);

/**
@brief Releases a style object. *style will be set to invalid by this function.
@param[in] style The style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if style is NULL
- \ref SU_ERROR_INVALID_INPUT if *style does not refer to a valid object
*/
LO_RESULT LOStyleRelease(LOStyleRef* style);

/**
@brief Copies the style attributes from one style object to another.
@param[in]  src_style The source style object.
@param[out] dst_style The destination style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if src_style does not refer to a valid object
- \ref SU_ERROR_INVALID_OUTPUT if dst_style does not refer to a valid object
*/
LO_RESULT LOStyleCopy(LOStyleRef src_style, LOStyleRef dst_style);

/**
@brief Gets whether or not a style has a stroke.
@param[in]  style      The style object.
@param[out] is_stroked Whether there is a stroke or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_stroked is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetStroked(LOStyleRef style, bool* is_stroked);

/**
@brief Sets whether or not a style has a stroke.
@param[in] style      The style object.
@param[in] is_stroked Whether there should be a stroke or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
*/
LO_RESULT LOStyleSetStroked(LOStyleRef style, bool is_stroked);

/**
@brief Gets the stroke width of a style.
@param[in]  style        The style object.
@param[out] stroke_width The stroke width.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if stroke_width is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetStrokeWidth(LOStyleRef style, double* stroke_width);

/**
@brief Sets the stroke width of a style.
@param[in] style        The style object.
@param[in] stroke_width The stroke width.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if stroke_width is less than 0.0
*/
LO_RESULT LOStyleSetStrokeWidth(LOStyleRef style, double stroke_width);

/**
@brief Gets the stroke color of a style.
@param[in]  style        The style object.
@param[out] stroke_color The stroke color.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if stroke_color is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetStrokeColor(LOStyleRef style, SUColor* stroke_color);

/**
@brief Sets the stroke color of a style.
@param[in] style        The style object.
@param[in] stroke_color The stroke color.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
*/
LO_RESULT LOStyleSetStrokeColor(LOStyleRef style, SUColor stroke_color);

/**
@brief Gets the stroke join style of a style.
@param[in]  style     The style object.
@param[out] join_type The join style.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if join_type is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetStrokeJoinStyle(LOStyleRef style, LOStrokeJoinStyle* join_type);

/**
@brief Sets the stroke join style of a style.
@param[in] style     The style object.
@param[in] join_type The join style.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if join_type is invalid
*/
LO_RESULT LOStyleSetStrokeJoinStyle(LOStyleRef style, LOStrokeJoinStyle join_type);

/**
@brief Gets the stroke cap style of a style.
@param[in]  style    The style object.
@param[out] cap_type The cap style.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if cap_type is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetStrokeCapStyle(LOStyleRef style, LOStrokeCapStyle* cap_type);

/**
@brief Sets the stroke cap style of a style.
@param[in] style     The style object.
@param[in] cap_type  The cap style.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if cap_type is invalid
*/
LO_RESULT LOStyleSetStrokeCapStyle(LOStyleRef style, LOStrokeCapStyle cap_type);

/**
@brief Gets the stroke pattern of a style.
@param[in]  style          The style object.
@param[out] stroke_pattern The stroke pattern.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if stroke_pattern is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetStrokePattern(LOStyleRef style, LOStrokePattern* stroke_pattern);

/**
@brief Sets the stroke pattern of a style.
@param[in] style          The style object.
@param[in] stroke_pattern The stroke pattern.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if stroke_pattern is invalid
*/
LO_RESULT LOStyleSetStrokePattern(LOStyleRef style, LOStrokePattern stroke_pattern);

/**
@brief Gets the stroke pattern scale of a style.
@param[in]  style         The style object.
@param[out] pattern_scale The stroke pattern scale.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if pattern_scale is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetStrokePatternScale(LOStyleRef style, double* pattern_scale);

/**
@brief Sets the stroke pattern scale of a style.
@param[in] style         The style object.
@param[in] pattern_scale The stroke pattern scale.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if pattern_scale is less than 0.001 or greater than
  9999.0
*/
LO_RESULT LOStyleSetStrokePatternScale(LOStyleRef style, double pattern_scale);

/**
@brief Gets whether or not a style has a solid color fill.
@param[in]  style        The style object.
@param[out] solid_filled Whether there is a solid color fill or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if solid_filled is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetSolidFilled(LOStyleRef style, bool* solid_filled);

/**
@brief Sets whether or not a style has a solid color fill.
@param[in] style     The style object.
@param[in] solid_filled Whether there should be a solid color fill or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
*/
LO_RESULT LOStyleSetSolidFilled(LOStyleRef style, bool solid_filled);

/**
@brief Gets the solid fill color of a style.
@param[in]  style      The style object.
@param[out] fill_color The solid fill color.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if fill_color is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetSolidFillColor(LOStyleRef style, SUColor* fill_color);

/**
@brief Sets the solid fill color of a style.
@param[in] style      The style object.
@param[in] fill_color The solid fill color.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
*/
LO_RESULT LOStyleSetSolidFillColor(LOStyleRef style, SUColor fill_color);

/**
@brief Gets whether there is a pattern fill or not of a style.
@param[in]  style          The style object.
@param[out] pattern_filled Whether there is a pattern fill or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if pattern_filled is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetPatternFilled(LOStyleRef style, bool* pattern_filled);

/**
@brief Sets whether or not a style has a pattern fill.
@param[in] style          The style object.
@param[in] pattern_filled Whether there should be a pattern fill or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
*/
LO_RESULT LOStyleSetPatternFilled(LOStyleRef style, bool pattern_filled);

/**
@brief Gets the pattern fill image path of a style. This is the file path for
       the pattern fill image file reference.
@param[in]  style      The style object.
@param[out] image_path The pattern fill image path.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if image_path is NULL
- \ref SU_ERROR_INVALID_OUTPUT if *image_path does not refer to a valid string
  object
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetPatternFillImagePath(LOStyleRef style, SUStringRef* image_path);

/**
@brief Sets the pattern fill image path of a style. This is the file path for
       the pattern fill image file reference.
@param[in] style      The style object.
@param[in] image_path The pattern fill image path.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if image_path is NULL
- \ref SU_ERROR_SERIALIZATION if the image file could not be loaded
*/
LO_RESULT LOStyleSetPatternFillImagePath(LOStyleRef style, const char* image_path);

/**
@brief Gets the rotation angle applied to the pattern fill of a style.
@param[in]  style    The style object.
@param[out] rotation The rotation angle applied to the pattern fill, in degrees.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if rotation is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetPatternFillRotation(LOStyleRef style, double* rotation);

/**
@brief Sets the rotation angle applied to the pattern fill of a style.
@param[in]  style    The style object.
@param[out] rotation The rotation angle to apply to the pattern fill, in degrees.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
  attribute
*/
LO_RESULT LOStyleSetPatternFillRotation(LOStyleRef style, double rotation);

/**
@brief Gets the scale applied to the pattern fill of a style.
@param[in]  style The style object.
@param[out] scale The scale applied to the pattern fill.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if scale is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetPatternFillScale(LOStyleRef style, double* scale);

/**
@brief Sets the scale applied to the pattern fill of a style.
@param[in]  style The style object.
@param[out] scale The scale to apply to the pattern fill.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
  attribute
- \ref SU_ERROR_OUT_OF_RANGE if scale is less than 0.001 or greater than
  9999.0
*/
LO_RESULT LOStyleSetPatternFillScale(LOStyleRef style, double scale);

/**
@brief Gets the origin for the pattern fill of a style.
@param[in]  style  The style object.
@param[out] origin The paper space origin for the pattern fill.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if origin is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetPatternFillOrigin(LOStyleRef style, LOPoint2D* origin);

/**
@brief Sets the origin for the pattern fill of a style.
@param[in]  style  The style object.
@param[out] origin The paper space origin for the pattern fill.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if origin is NULL
*/
LO_RESULT LOStyleSetPatternFillOrigin(LOStyleRef style, const LOPoint2D* origin);

/**
@brief Gets the text font name of a style.
@param[in]  style  The style object.
@param[out] family The name of the font.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if family is NULL
- \ref SU_ERROR_INVALID_OUTPUT if family does not refer to a valid object
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetTextFontFamily(LOStyleRef style, SUStringRef* family);

/**
@brief Sets the text font name of a style.
@param[in] style  The style object.
@param[in] family The name of the new font to use.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if family is NULL
*/
LO_RESULT LOStyleSetTextFontFamily(LOStyleRef style, const char* family);

/**
@brief Gets whether or not text should be bold for a style.
@param[in]  style   The style object.
@param[out] is_bold Whether text will be bolded or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_bold is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetTextBold(LOStyleRef style, bool* is_bold);

/**
@brief Sets whether or not text should be bold for a style.
@param[in]  style   The style object.
@param[out] is_bold Whether text should be bolded or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
*/
LO_RESULT LOStyleSetTextBold(LOStyleRef style, bool is_bold);

/**
@brief Gets whether or not text should be italicized for a style.
@param[in]  style    The style object.
@param[out] is_italic Whether text will be italic or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_italic is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetTextItalic(LOStyleRef style, bool* is_italic);

/**
@brief Sets whether or not text should be italicized for a style.
@param[in]  style     The style object.
@param[out] is_italic Whether text should be italic or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
*/
LO_RESULT LOStyleSetTextItalic(LOStyleRef style, bool is_italic);

/**
@brief Gets the font size of a style.
@param[in]  style          The style object.
@param[out] size_in_points The font size in points.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if size_in_points is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetFontSize(LOStyleRef style, double* size_in_points);

/**
@brief Sets the font size of a style. On Windows only, this font size will be
       truncated to an integer value.
@param[in] style          The style object.
@param[in] size_in_points The font size in points.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if size_in_points is less than 1.0
*/
LO_RESULT LOStyleSetFontSize(LOStyleRef style, double size_in_points);

/**
@brief Gets the text color of a style.
@param[in]  style The style object.
@param[out] color The color of the text.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetTextColor(LOStyleRef style, SUColor* color);

/**
@brief Sets the text color of a style.
@param[in] style The style object.
@param[in] color The new text color to use.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
*/
LO_RESULT LOStyleSetTextColor(LOStyleRef style, SUColor color);

/**
@brief Gets the text underline type of a style.
@param[in]  style          The style object.
@param[out] underline_type The type of underline for text.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if underline_type is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetTextUnderline(LOStyleRef style, LOTextUnderline* underline_type);

/**
@brief Sets the text underline type of a style.
@param[in]  style          The style object.
@param[out] underline_type The type of underline to use for text.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if underline_type is invalid
*/
LO_RESULT LOStyleSetTextUnderline(LOStyleRef style, LOTextUnderline underline_type);

/**
@brief Gets the text elevation type (normal, superscript, or subscript) of a
       style.
@param[in]  style          The style object.
@param[out] elevation_type The text elevation type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if elevation_type is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetTextElevation(LOStyleRef style, LOTextElevation* elevation_type);

/**
@brief Sets the text elevation type of a style.
@param[in] style          The style object.
@param[in] elevation_type The new text elevation type to use.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if elevation_type is invalid
*/
LO_RESULT LOStyleSetTextElevation(LOStyleRef style, LOTextElevation elevation_type);

/**
@brief Gets the text alignment type of a style.
@param[in]  style          The style object.
@param[out] alignment_type The alignment type for text.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if alignment_type is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetTextAlignment(LOStyleRef style, LOTextAlignment* alignment_type);

/**
@brief Sets the text alignment type of a style.
@param[in] style          The style object.
@param[in] alignment_type The new text alignment type to use.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if alignment_type is invalid
*/
LO_RESULT LOStyleSetTextAlignment(LOStyleRef style, LOTextAlignment alignment_type);

/**
@brief Gets the text alignment type of a style.
@param[in]  style       The style object.
@param[out] anchor_type The anchor type for text.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if anchor_type is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetTextAnchor(LOStyleRef style, LOTextAnchor* anchor_type);

/**
@brief Sets the text alignment type of a style.
@param[in] style       The style object.
@param[in] anchor_type The new text anchor type to use.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if anchor_type is invalid
*/
LO_RESULT LOStyleSetTextAnchor(LOStyleRef style, LOTextAnchor anchor_type);

/**
@brief Gets the dimension rotation alignment type of a style.
@param[in]  style     The style object.
@param[out] alignment The alignment.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if alignment is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetDimensionRotationAlignment(
    LOStyleRef style, LODimensionRotationAlignment* alignment);

/**
@brief Sets the dimension rotation alignment type of a style.
@param[in] style     The style object.
@param[in] alignment The new alignment to use.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if alignment is not a valid value
*/
LO_RESULT LOStyleSetDimensionRotationAlignment(
    LOStyleRef style, LODimensionRotationAlignment alignment);

/**
@brief Gets the dimension vertical alignment setting of a style.
@param[in]  style     The style object.
@param[out] alignment The alignment.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if alignment is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetDimensionVerticalAlignment(
    LOStyleRef style, LODimensionVerticalAlignment* alignment);

/**
@brief Sets the dimension vertical alignment setting of a style. You can not
       set the alignment to \ref LODimensionVerticalAlignment_Offset because it
       requires a relative vector for the position.
@param[in] style     The style object.
@param[in] alignment The new alignment to use.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if alignment is not a valid value
*/
LO_RESULT LOStyleSetDimensionVerticalAlignment(
    LOStyleRef style, LODimensionVerticalAlignment alignment);

/**
@brief Gets the dimension units and precision of a style.
@param[in]  style     The style object.
@param[out] units     The units setting.
@param[out] precision The units precision. This is expressed as a value in the
                      current units.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if units is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if precision is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetDimensionUnits(LOStyleRef style, LODimensionUnits* units, double* precision);

/**
@brief Sets the dimension units and precision of a style.
@param[in]  style     The style object.
@param[out] units     The units setting for the document.
@param[out] precision The units precision. This is expressed as a value in the
                      specified units. LayOut only allows for a finite set of
                      precision values for each units setting, so it will set
                      the precision to the closest valid setting for the
                      specified units. See the "Dimension Inspector" for a
                      reference of the available precisions for each units
                      setting.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if units is invalid
*/
LO_RESULT LOStyleSetDimensionUnits(LOStyleRef style, LODimensionUnits units, double precision);

/**
@brief Gets the dimension shall sppress unit display attribute of a style.
@param[in]  style         The style object.
@param[out] is_suppressed Is the dimension suppressing unit display.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_suppressed is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
  attribute
*/
LO_RESULT LOStyleGetSuppressDimensionUnits(LOStyleRef style, bool* is_suppressed);

/**
@brief Sets the dimension shall sppress unit display attribute of a style.
@param[in] style         The style object.
@param[in] is_suppressed Is the dimension suppressing unit display.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
*/
LO_RESULT LOStyleSetSuppressDimensionUnits(LOStyleRef style, bool is_suppressed);





/**
@brief Gets the maximum number of widths that may be returned by
       LOStrokePatternGetWidths. Use this to determine the size of the array to
       pass to LOStrokePatternGetWidths.
@param[out] num_widths     The maximum number of widths.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if num_widths is NULL
*/
LO_RESULT LOStrokePatternGetMaximumNumberOfWidths(size_t* num_widths);

/**
@brief Gets the widths of the alternating dashes and gaps for the given stroke
       pattern. These widths are expressed in paper space inches, and may be
       scaled (see \ref LOStyleGetStrokePatternScale). Use \ref
       LOStrokePatternGetMaximumNumberOfWidths to determine an adequate length
       for the widths array.
@param[in]  stroke_pattern The stroke pattern.
@param[in]  len            The maximum number of widths to return.
@param[out] widths         The widths for the stroke pattern.
@param[out] num_widths     The number of widths returned.

@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_OUT_OF_RANGE if stroke_pattern is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if widths is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if num_widths is NULL
*/
LO_RESULT LOStrokePatternGetWidths(
    LOStrokePattern stroke_pattern, size_t len, double widths[], size_t* num_widths);

/**
@brief Gets the the start arrow type of a style.
@param[in]  style The style object.
@param[out] type  The arrow type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if type is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
*/
LO_RESULT LOStyleGetStartArrowType(LOStyleRef style, LOArrowType* type);

/**
@brief Sets the the start arrow type of a style.
@param[in] style The style object.
@param[in] type  The arrow type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if type is invalid
*/
LO_RESULT LOStyleSetStartArrowType(LOStyleRef style, LOArrowType type);

/**
@brief Gets the the start arrow size of a style.
@param[in]  style The style object.
@param[out] size  The arrow size.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if size is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
*/
LO_RESULT LOStyleGetStartArrowSize(LOStyleRef style, double* size);

/**
@brief Sets the the start arrow size of a style.
@param[in] style The style object.
@param[in] size  The arrow size.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if size is less than 0.25
*/
LO_RESULT LOStyleSetStartArrowSize(LOStyleRef style, double size);

/**
@brief Gets the end arrow type of a style.
@param[in]  style The style object.
@param[out] type  The arrow type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if type is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
*/
LO_RESULT LOStyleGetEndArrowType(LOStyleRef style, LOArrowType* type);

/**
@brief Sets the end arrow type of a style.
@param[in] style The style object.
@param[in] type  The arrow type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if type is invalid
*/
LO_RESULT LOStyleSetEndArrowType(LOStyleRef style, LOArrowType type);

/**
@brief Gets the end arrow size of a style.
@param[in]  style The style object.
@param[out] size  The arrow size.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if size is NULL
- \ref SU_ERROR_NO_DATA if style doesn't specify a value for this style
*/
LO_RESULT LOStyleGetEndArrowSize(LOStyleRef style, double* size);

/**
@brief Sets the end arrow size of a style.
@param[in] style The style object.
@param[in] size  The arrow size.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if size is less than 0.25
*/
LO_RESULT LOStyleSetEndArrowSize(LOStyleRef style, double size);

/**
@brief Gets whether or not an arrow type is filled in.
@param[in]  type   The arrow type.
@param[out] filled Whether the arrow type is filled in or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if filled is NULL
- \ref SU_ERROR_OUT_OF_RANGE if type is invalid
*/
LO_RESULT LOStyleIsArrowTypeFilled(LOArrowType type, bool* filled);

/**
@brief Adds a style to apply to an entity's sub-entity. This would be used to
       set the arrow head type for extension lines of a dimension, for example.
@param[in] style     The style to contain the sub entity style.
@param[in] type      The type of sub entity the sub style is for.
@param[in] sub_style The style to set as a sub entity style.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if sub_style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if type is invalid
*/
LO_RESULT LOStyleSetSubEntityStyle(LOStyleRef style, LOSubEntityType type, LOStyleRef sub_style);

/**
@brief Gets the style settings for a sub entity from a style. This would be used
       to get the current style of a dimension's text, for example.
@param[in] style     The style containing the sub entity style.
@param[in] type      The type of sub entity the sub style is for.
@param[in] sub_style The sub entity style.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if sub_style is NULL
- \ref SU_ERROR_INVALID_OUTPUT if sub_style does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if type is invalid
*/
LO_RESULT LOStyleGetSubEntityStyle(LOStyleRef style, LOSubEntityType type, LOStyleRef* sub_style);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus
#endif  // LAYOUT_MODEL_STYLE_H_
