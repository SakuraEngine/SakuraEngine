// Copyright 2013-2020 Trimble Inc.  All Rights Reserved

/**
 * @file
 * @brief Interfaces for SUAttributeDictionaryRef.
 */
#ifndef SKETCHUP_MODEL_ATTRIBUTE_DICTIONARY_H_
#define SKETCHUP_MODEL_ATTRIBUTE_DICTIONARY_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>
#include <SketchUpAPI/model/typed_value.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Creates an attributes dictionary object.
@since SketchUp 2018 M0, API 6.0
@param[out] dictionary The attributes dictionary object created.
@param[in]  name       The name of the attribute dictionary. Assumed to be UTF-8
                       encoded.
@related SUAttributeDictionaryRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if dictionary is NULL
- \ref SU_ERROR_OVERWRITE_VALID if dictionary already references a valid object
*/
SU_RESULT SUAttributeDictionaryCreate(SUAttributeDictionaryRef* dictionary, const char* name);

/**
@brief Releases an attributes dictionary object and its associated attributes.
       If this dictionary has a parent, it will be removed from it.
@since SketchUp 2018 M0, API 6.0
@param[in,out] dictionary The attributes dictionary object.
@related SUAttributeDictionaryRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if dictionary is NULL
- \ref SU_ERROR_INVALID_INPUT if dictionary does not reference a valid object
*/
SU_RESULT SUAttributeDictionaryRelease(SUAttributeDictionaryRef* dictionary);

/**
@brief Converts from an \ref SUAttributeDictionaryRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@since SketchUp 2014, API 2.0
@param[in] dictionary The attribute dictionary object.
@related SUAttributeDictionaryRef
@return
- The converted \ref SUEntityRef if dictionary is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUAttributeDictionaryToEntity(SUAttributeDictionaryRef dictionary);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUAttributeDictionaryRef.
       This is essentially a downcast operation so the given \ref SUEntityRef
       must be convertible to an \ref SUAttributeDictionaryRef.
@since SketchUp 2014, API 2.0
@param[in] entity The given entity reference.
@related SUAttributeDictionaryRef
@return
- The converted \ref SUAttributeDictionaryRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUAttributeDictionaryRef SUAttributeDictionaryFromEntity(SUEntityRef entity);

/**
@struct SUAttributeDictionaryRef
@extends SUEntityRef
@brief  A dictionary type with SUStringRef objects as keys and SUTypedValueRef
        objects as values.
*/

/**
@brief Retrieves the name of an attribute dictionary object.
@param[in]  dictionary The attribute dictionary object.
@param[out] name       The name retrieved.
@related SUAttributeDictionaryRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dictionary is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUAttributeDictionaryGetName(SUAttributeDictionaryRef dictionary, SUStringRef* name);

/**
@brief Inserts a key-value pair into an attribute dictionary object.
@param[in] dictionary The attribute dictionary object.
@param[in] key        The key of the key-value pair. Assumed to be UTF-8
                      encoded.
@param[in] value_in   The value of the key-value pair.
@related SUAttributeDictionaryRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dictionary or value_in is an invalid object.
- \ref SU_ERROR_NULL_POINTER_INPUT if key is NULL
- \ref SU_ERROR_INVALID_OPERATION if dictionary is read-only.
*/
SU_RESULT SUAttributeDictionarySetValue(
    SUAttributeDictionaryRef dictionary, const char* key, SUTypedValueRef value_in);

/**
@brief Retrieves the value associated with a given key from an attribute
       dictionary.
@param[in] dictionary The attribute dictionary object.
@param[in] key        The key of the key-value pair. Assumed to be UTF-8
                      encoded.
@param[out] value_out The value retrieved. Must be a valid object, i.e.
                      must have been allocated via SUTypedValueCreate().
@related SUAttributeDictionaryRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dictionary is an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if key is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if value_out is NULL
- \ref SU_ERROR_INVALID_OUTPUT if value_out is an invalid object
- \ref SU_ERROR_NO_DATA if there is no value associated with the given key in
  the dictionary
*/
SU_RESULT SUAttributeDictionaryGetValue(
    SUAttributeDictionaryRef dictionary, const char* key, SUTypedValueRef* value_out);

/**
@brief Retrieves the number of keys in an attribute dictionary object.
@param[in]  dictionary The attribute dictionary object.
@param[out] count      The number of keys.
@related SUAttributeDictionaryRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dictionary is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUAttributeDictionaryGetNumKeys(SUAttributeDictionaryRef dictionary, size_t* count);

/**
@brief Retrieves the array of keys of an attribute dictionary object.
@param[in]  dictionary The attribute dictionary object.
@param[in]  len        The number of keys to retrieve.
@param[out] keys       The keys retrieved.
@param[out] count      The number of keys retrieved.
@related SUAttributeDictionaryRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dictionary is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if keys or count is NULL
*/
SU_RESULT SUAttributeDictionaryGetKeys(
    SUAttributeDictionaryRef dictionary, size_t len, SUStringRef keys[], size_t* count);

#ifdef __cplusplus
}  //  extern "C" {
#endif

#endif  // SKETCHUP_MODEL_ATTRIBUTE_DICTIONARY_H_
