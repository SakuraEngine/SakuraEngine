// Copyright 2016-2020 Trimble Inc. All Rights Reserved

/**
 * @file
 * @brief Interfaces for SUClassificationAttributeRef.
 */
#ifndef SKETCHUP_MODEL_CLASSIFICATION_ATTRIBUTE_H_
#define SKETCHUP_MODEL_CLASSIFICATION_ATTRIBUTE_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUClassificationAttributeRef
@brief  References attribute data about a classified component.
*/

/**
@brief Retrieves the value of the attribute.
@since SketchUp 2017, API 5.0
@param[in]  attribute The classification attribute object.
@param[out] value     The value of the attribute.
@related SUClassificationAttributeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if attribute is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if value is NULL
- \ref SU_ERROR_INVALID_OUTPUT if value does not point to a valid \ref
  SUTypedValueRef object
*/
SU_RESULT SUClassificationAttributeGetValue(
    SUClassificationAttributeRef attribute, SUTypedValueRef* value);

/**
@brief Retrieves the path to the attribute.
@since SketchUp 2017, API 5.0
@param[in]  attribute The classification attribute object.
@param[out] path      The attribute name as it should be displayed to the
                         user.
@related SUClassificationAttributeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if attribute is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if path is NULL
- \ref SU_ERROR_INVALID_OUTPUT if path does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUClassificationAttributeGetPath(
    SUClassificationAttributeRef attribute, SUStringRef* path);

/**
@brief Retrieves the number of children setting of the attribute.
@since SketchUp 2017, API 5.0
@param[in]  attribute The classification attribute object.
@param[out] count     The number of children this attribute contains.
@related SUClassificationAttributeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if attribute is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUClassificationAttributeGetNumChildren(
    SUClassificationAttributeRef attribute, size_t* count);

/**
@brief Retrieves the child attribute at the given index.
@since SketchUp 2017, API 5.0
@param[in]  attribute The classification attribute object.
@param[in]  index     The index of the child attribute to get.
@param[out] child     The child attribute retrieved.
@related SUClassificationAttributeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if attribute is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if child is NULL
*/
SU_RESULT SUClassificationAttributeGetChild(
    SUClassificationAttributeRef attribute, size_t index, SUClassificationAttributeRef* child);

#ifdef __cplusplus
}  //  extern "C" {
#endif

#endif  // SKETCHUP_MODEL_CLASSIFICATION_ATTRIBUTE_H_
