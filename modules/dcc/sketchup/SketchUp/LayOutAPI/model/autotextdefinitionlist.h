// Copyright 2017 Trimble Inc. All Rights Reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_AUTOTEXTDEFINITIONLIST_H_
#define LAYOUT_MODEL_AUTOTEXTDEFINITIONLIST_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOAutoTextDefinitionListRef
@brief References an ordered, indexable list of \ref LOAutoTextDefinitionRef
       objects.
@since LayOut 2018, API 3.0
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Creates a new empty auto-text definition list object.
@since LayOut 2018, API 3.0
@param[out] auto_text_list An empty auto-text definition list object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if auto_text_list is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *auto_text_list already refers to a valid
  object
*/
LO_RESULT LOAutoTextDefinitionListCreate(LOAutoTextDefinitionListRef* auto_text_list);

/**
@brief Releases an auto-text definition list object. *auto_text_list will be set
       to invalid by this function.
@since LayOut 2018, API 3.0
@param[in] auto_text_list The auto-text definition list object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if auto_text_list does not refer to a valid
  object
- \ref SU_ERROR_INVALID_INPUT if *auto_text_list does not refer to a valid object
*/
LO_RESULT LOAutoTextDefinitionListRelease(LOAutoTextDefinitionListRef* auto_text_list);

/**
@brief Gets the number of auto-text definitions in this list.
@since LayOut 2018, API 3.0
@param[in]  auto_text_list The auto-text definition list object.
@param[out] num_auto_texts The number of auto-text definition objects in the list.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if auto_text_list does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if num_auto_texts is NULL
*/
LO_RESULT LOAutoTextDefinitionListGetNumberOfAutoTextDefinitions(
    LOAutoTextDefinitionListRef auto_text_list, size_t* num_auto_texts);

/**
@brief Gets the auto-text definition at the specified index.
@since LayOut 2018, API 3.0
@param[in]  auto_text_list The auto-text definition list object.
@param[in]  index          The index of the auto-text definition to get.
@param[out] auto_text      The auto-text definition object at index in the list.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if auto_text_list does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is out of range for this list
- \ref SU_ERROR_NULL_POINTER_OUTPUT if auto_text is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *auto_text already refers to a valid object
*/
LO_RESULT LOAutoTextDefinitionListGetAutoTextDefinitionAtIndex(
    LOAutoTextDefinitionListRef auto_text_list, size_t index, LOAutoTextDefinitionRef* auto_text);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_auto_text_list_H_
