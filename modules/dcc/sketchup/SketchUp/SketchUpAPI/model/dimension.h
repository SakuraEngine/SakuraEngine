// Copyright 2016 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUDimensionRef.
 */
#ifndef SKETCHUP_MODEL_DIMENSION_H_
#define SKETCHUP_MODEL_DIMENSION_H_

#include <SketchUpAPI/color.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>
#include <SketchUpAPI/model/arrow_type.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUDimensionRef
@extends SUDrawingElementRef
@brief  A dimension entity reference.
@since SketchUp 2017, API 5.0
*/

/**
@enum SUDimensionType
@brief Indicates the supported dimension types
*/
enum SUDimensionType {
  SUDimensionType_Invalid = 0,
  SUDimensionType_Linear,
  SUDimensionType_Radial
};

/**
@brief Converts from an \ref SUDimensionRef to an \ref SUEntityRef. This is
       essentially an upcast operation.
@since SketchUp 2017, API 5.0
@param[in] dimension The given dimension reference.
@related SUDimensionRef
@return
- The converted \ref SUEntityRef if dimension is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUDimensionToEntity(SUDimensionRef dimension);

/**
@brief Converts from an SUEntityRef to an \ref SUDimensionRef. This is
       essentially a downcast operation so the given SUEntityRef must be
       convertible to an \ref SUDimensionRef.
@since SketchUp 2017, API 5.0
@param[in] entity The given entity reference.
@related SUDimensionRef
@return
- The converted \ref SUDimensionRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDimensionRef SUDimensionFromEntity(SUEntityRef entity);

/**
@brief Converts from an \ref SUDimensionRef to an \ref SUDrawingElementRef.
       This is essentially an upcast operation.
@since SketchUp 2017, API 5.0
@param[in] dimension The given dimension reference.
@related SUDimensionRef
@return
- The converted \ref SUDrawingElementRef if dimension is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDrawingElementRef SUDimensionToDrawingElement(SUDimensionRef dimension);

/**
@brief Converts from an SUDrawingElementRef to an \ref SUDimensionRef. This is
       essentially a downcast operation so the given SUDrawingElementRef must
       be convertible to an \ref SUDimensionRef.
@since SketchUp 2017, API 5.0
@param[in] element The given drawing element reference.
@related SUDimensionRef
@return
- The converted \ref SUDimensionRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDimensionRef SUDimensionFromDrawingElement(SUDrawingElementRef element);

/**
@brief Retrieves an enum value indicating the dimension type (linear or radial).
@since SketchUp 2017, API 5.0
@param[in]  dimension The dimension object.
@param[out] type      The dimension type enum value retrieved.
@related SUDimensionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if type is NULL
*/
SU_RESULT SUDimensionGetType(SUDimensionRef dimension, enum SUDimensionType* type);

/**
@brief Retrieves the text of a dimension object.
@since SketchUp 2017, API 5.0
@param[in]  dimension The dimension object.
@param[out] text      The name retrieved.
@related SUDimensionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if text is NULL
- \ref SU_ERROR_INVALID_OUTPUT if text does not point to a valid \ref
       SUStringRef object
*/
SU_RESULT SUDimensionGetText(SUDimensionRef dimension, SUStringRef* text);

/**
@brief Sets the text of a dimension object.
@since SketchUp 2017, API 5.0
@param[in] dimension The dimension object.
@param[in] text      The text to be set. Assumed to be UTF-8 encoded.
@related SUDimensionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if text is NULL
*/
SU_RESULT SUDimensionSetText(SUDimensionRef dimension, const char* text);

/**
@brief Retrieves the plane of a dimension object.
@since SketchUp 2017, API 5.0
@param[in]  dimension The dimension object.
@param[out] plane     The 3d plane retrieved.
@related SUDimensionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if plane is NULL
*/
SU_RESULT SUDimensionGetPlane(SUDimensionRef dimension, struct SUPlane3D* plane);

/**
@brief Retrieves a boolean indicating if the dimension text is 3D.
@since SketchUp 2017, API 5.0
@param[in]  dimension  The dimension object.
@param[out] is_text_3d The flag value retrieved.
@related SUDimensionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_text_3d is NULL
*/
SU_RESULT SUDimensionGetText3D(SUDimensionRef dimension, bool* is_text_3d);

/**
@brief Sets a boolean indicating whether the dimension text is 3D.
@since SketchUp 2017, API 5.0
@param[in] dimension  The dimension object.
@param[in] is_text_3d The flag to be set.
@related SUDimensionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
*/
SU_RESULT SUDimensionSetText3D(SUDimensionRef dimension, bool is_text_3d);

/**
@brief Retrieves an enum value indicating the dimension's arrow type.
@since SketchUp 2017, API 5.0
@param[in]  dimension The dimension object.
@param[out] type      The arrow type enum value retrieved.
@related SUDimensionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if type is NULL
*/
SU_RESULT SUDimensionGetArrowType(SUDimensionRef dimension, enum SUArrowType* type);

/**
@brief Sets the dimension's arrow type.
@since SketchUp 2017, API 5.0
@param[in] dimension The dimension object.
@param[in] type      The arrow type to be set.
@related SUDimensionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE if type is not a supported value
*/
SU_RESULT SUDimensionSetArrowType(SUDimensionRef dimension, enum SUArrowType type);

/**
@brief Get the dimension's font reference.
@since SketchUp 2019, API 7.0
@param[in]  dimension The dimension object.
@param[out] font      The font retrieved.
@related SUDimensionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if font is NULL
*/
SU_RESULT SUDimensionGetFont(SUDimensionRef dimension, SUFontRef* font);

/**
@brief Sets the dimension's font from a font reference.
@since SketchUp 2019, API 7.0
@param[in] dimension The dimension object.
@param[in] font      The font to be set.
@related SUDimensionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dimension or font is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if font is NULL
*/
SU_RESULT SUDimensionSetFont(SUDimensionRef dimension, SUFontRef font);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_DIMENSION_H_
