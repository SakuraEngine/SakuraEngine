// Copyright 2015 Trimble Inc.  All Rights Reserved

/**
 * @file
 * @brief Interfaces for SUDynamicComponentInfoRef.
 */
#ifndef SKETCHUP_MODEL_DYNAMIC_COMPONENT_INFO_H_
#define SKETCHUP_MODEL_DYNAMIC_COMPONENT_INFO_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUDynamicComponentInfoRef
@brief  References an object with information about a dynamic component.
*/

/**
@brief Releases the DC info. DC info objects are created from component instance
       using SUComponentInstanceCreateDCInfo(), and must be released using
       this function. This function also invalidates the given DC info.
@since SketchUp 2016, API 4.0
@param[in,out]  dc_info The dynamic component info object.
@related SUDynamicComponentInfoRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if dc_info is NULL
- \ref SU_ERROR_INVALID_INPUT if dc_info is not a valid object
*/
SU_RESULT SUDynamicComponentInfoRelease(SUDynamicComponentInfoRef* dc_info);

/**
@brief Retrieves the number of dynamic component attributes.
@param[in]  dc_info The dynamic component info object.
@param[out] count   The number of attributes.
@related SUDynamicComponentInfoRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dc_info is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUDynamicComponentInfoGetNumDCAttributes(
    SUDynamicComponentInfoRef dc_info, size_t* count);

/**
@brief Retrieves the dynamic component attributes.
@param[in]  dc_info    The dynamic component info object.
@param[in]  len        The number of attributes to retrieve.
@param[out] attributes The attributes retrieved.
@param[out] count      The number of attributes retrieved.
@related SUDynamicComponentInfoRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dc_info is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if attributes or count is NULL
*/
SU_RESULT SUDynamicComponentInfoGetDCAttributes(
    SUDynamicComponentInfoRef dc_info, size_t len, SUDynamicComponentAttributeRef attributes[],
    size_t* count);

#ifdef __cplusplus
}  //  extern "C" {
#endif

#endif  // SKETCHUP_MODEL_DYNAMIC_COMPONENT_INFO_H_
