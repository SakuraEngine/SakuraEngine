// Copyright 2015 Trimble Inc.  All Rights Reserved

/**
 * @file
 * @brief Interfaces for SUDynamicComponentAttributeRef.
 */
#ifndef SKETCHUP_MODEL_DYNAMIC_COMPONENT_ATTRIBUTE_H_
#define SKETCHUP_MODEL_DYNAMIC_COMPONENT_ATTRIBUTE_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUDynamicComponentAttributeRef
@brief  References attribute data about a dynamic component.
*/

/**
@brief Gets the name of the attribute.
@since SketchUp 2016, API 4.0
@param[in]  attribute The dynamic component attribute object.
@param[out] name      The internal name of the attribute.
@related SUDynamicComponentAttributeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if attribute is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUDynamicComponentAttributeGetName(
    SUDynamicComponentAttributeRef attribute, SUStringRef* name);

/**
@brief Gets the display name of the attribute.
@since SketchUp 2016, API 4.0
@param[in]  attribute    The dynamic component attribute object.
@param[out] display_name The attribute name as it should be displayed to the
                         user.
@related SUDynamicComponentAttributeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if attribute is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if display_name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if display_name does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUDynamicComponentAttributeGetDisplayName(
    SUDynamicComponentAttributeRef attribute, SUStringRef* display_name);

/**
@brief Gets the visibility setting of the attribute.
@since SketchUp 2016, API 4.0
@param[in]  attribute  The dynamic component attribute object.
@param[out] visible    Set to true if the attribute is visible to users and
                       false if it is not.
@related SUDynamicComponentAttributeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if attribute is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if visible is NULL
*/
SU_RESULT SUDynamicComponentAttributeGetVisibility(
    SUDynamicComponentAttributeRef attribute, bool* visible);

/**
@brief Gets the display value of the attribute.
@since SketchUp 2016, API 4.0
@param[in]  attribute     The dynamic component attribute object.
@param[out] display_value The value data for the attribute converted to a string
                          formatted as it should be displayed to the user.
@related SUDynamicComponentAttributeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if attribute is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if display_value is NULL
- \ref SU_ERROR_INVALID_OUTPUT if display_value does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUDynamicComponentAttributeGetDisplayValue(
    SUDynamicComponentAttributeRef attribute, SUStringRef* display_value);

#ifdef __cplusplus
}  //  extern "C" {
#endif

#endif  // SKETCHUP_MODEL_DYNAMIC_COMPONENT_ATTRIBUTE_H_
