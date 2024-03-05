// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_DICTIONARY_H_
#define LAYOUT_MODEL_DICTIONARY_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LODictionaryRef
@brief References a dictionary. A dictionary maps string keys to
       \ref LOTypedValueRef values.
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Creates an new empty dictionary object.
@param[out] dictionary The newly created dictionary object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if dictionary is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *dictionary already refers to a valid object
*/
LO_RESULT LODictionaryCreate(LODictionaryRef* dictionary);

/**
@brief Releases a dictionary object. *dictionary will be set to invalid by
       this function.
@param[in] dictionary The dictionary object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if dictionary is NULL
- \ref SU_ERROR_INVALID_INPUT if *dictionary references an invalid object
*/
LO_RESULT LODictionaryRelease(LODictionaryRef* dictionary);

/**
@brief Retrieves the value associated with a given key from a dictionary.
@param[in] dictionary The dictionary object.
@param[in] key        The key of the key-value pair. Assumed to be UTF-8
                      encoded.
@param[out] value     The value retrieved. Must be a valid object, i.e.
                      must have been created via \ref LOTypedValueCreate.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dictionary is an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if key is NULL
- \ref SU_ERROR_INVALID_OUTPUT if value is an invalid object
- \ref SU_ERROR_NO_DATA if there is no value associated with the given key in
  the dictionary
*/
LO_RESULT LODictionaryGetValue(LODictionaryRef dictionary, const char* key, LOTypedValueRef value);

/**
@brief Inserts a key-value pair into a dictionary object.
@param[in] dictionary The dictionary object.
@param[in] key        The key of the key-value pair. Assumed to be UTF-8
                      encoded.
@param[in] value      The value of the key-value pair.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dictionary or value is an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if key is NULL
*/
LO_RESULT LODictionarySetValue(LODictionaryRef dictionary, const char* key, LOTypedValueRef value);

/**
@brief Removes all of the key-value pairs from a dictionary.
@since LayOut 2018, API 3.0
@param[in] dictionary The dictionary object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dictionary is an invalid object
*/
LO_RESULT LODictionaryClear(LODictionaryRef dictionary);

/**
@brief Gets the number of entries in the dictionary.
@since LayOut 2018, API 3.0
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dictionary does not refer to a valid
  object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if out_number_of_keys is NULL
*/
LO_RESULT LODictionaryGetNumberOfKeys(LODictionaryRef dictionary, size_t* out_number_of_keys);

/**
@brief Gets all the keys in the dictionary.
@since LayOut 2018, API 3.0
@return
- \ref SU_ERROR_NONE on success.
- \ref SU_ERROR_INVALID_INPUT if dictionary refers to an invalid reference
- \ref SU_ERROR_OUT_OF_RANGE if key_array_length must be at least 1
- \ref SU_ERROR_NULL_POINTER_OUTPUT if out_key_array or
  out_number_of_keys_copied is NULL
- \ref SU_ERROR_INVALID_OUTPUT if keys contains one or more invalid objects
*/
LO_RESULT LODictionaryGetKeys(
    LODictionaryRef dictionary, size_t key_array_length, SUStringRef keys[],
    size_t* out_number_of_keys_copied);

/**
@brief Remove the entry in the dictionary for the given key.
@since LayOut 2018, API 3.0
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if dictionary or key refer to invalid references
- \ref SU_ERROR_NO_DATA if there is no value to delete for the given key
*/
LO_RESULT LODictionaryRemoveEntry(LODictionaryRef dictionary, const char* key);
#ifdef __cplusplus
}  // extern "C" {
#endif

#endif  // LAYOUT_MODEL_DICTIONARY_H_
