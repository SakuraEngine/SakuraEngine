// Copyright 2017 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUEntityListRef.
 */
#ifndef SKETCHUP_MODEL_ENTITY_LIST_H_
#define SKETCHUP_MODEL_ENTITY_LIST_H_

#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUEntityListRef
@brief  References an entity list.
@since SketchUp 2018, API 6.0
*/

/**
@brief Creates an entity list object.
@since SketchUp 2018, API 6.0
@param[in,out] list The entity list object to be created.
@related SUEntityListRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if list is NULL
- \ref SU_ERROR_OVERWRITE_VALID if list already references a valid object
*/
SU_RESULT SUEntityListCreate(SUEntityListRef* list);

/**
@brief Releases a list object.
@since SketchUp 2018, API 6.0
@param[in,out] list The list object to be released.
@related SUEntityListRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if list is NULL
- \ref SU_ERROR_INVALID_INPUT if list does not reference a valid object
*/
SU_RESULT SUEntityListRelease(SUEntityListRef* list);

/**
@brief Sets the iterator reference to the beginning of the list. The given
       iterator object must have been constructed using
       SUEntityListIteratorCreate(). The iterator must be released using
       SUEntityListIteratorRelease() when it is no longer needed.
@since SketchUp 2018, API 6.0
@param[in]  list     The list.
@param[out] iterator An iterator Ref reference the beginning of the list.
@related SUEntityListRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if list is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if iterator is NULL
- \ref SU_ERROR_INVALID_OUTPUT if *iterator is not a valid object
*/
SU_RESULT SUEntityListBegin(SUEntityListRef list, SUEntityListIteratorRef* iterator);

/**
@brief Gets the number of entities in the list.
@since SketchUp 2018, API 6.0
@param[in]  list  The list object.
@param[out] count The list size.
@related SUEntityListRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if list is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEntityListSize(SUEntityListRef list, size_t* count);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_ENTITY_LIST_H_
