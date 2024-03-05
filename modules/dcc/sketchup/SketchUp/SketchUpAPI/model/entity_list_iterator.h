// Copyright 2017 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUEntityListIteratorRef.
 */
#ifndef SKETCHUP_MODEL_ENTITY_LIST_ITERATOR_H_
#define SKETCHUP_MODEL_ENTITY_LIST_ITERATOR_H_

#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUEntityListIteratorRef
@brief  References an entity list iterator object.
@since SketchUp 2018, API 6.0
*/

/**
@brief Creates an entity list iterator object.
@since SketchUp 2018, API 6.0
@param[in,out] iterator The entity list iterator object to be created.
@related SUEntityListIteratorRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if iterator is NULL
- \ref SU_ERROR_OVERWRITE_VALID if iterator already references a valid object
*/
SU_RESULT SUEntityListIteratorCreate(SUEntityListIteratorRef* iterator);

/**
@brief Releases a iterator object.
@since SketchUp 2018, API 6.0
@param[in,out] iterator The iterator object to be released.
@related SUEntityListIteratorRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if iterator is NULL
- \ref SU_ERROR_INVALID_INPUT if iterator does not reference a valid object
*/
SU_RESULT SUEntityListIteratorRelease(SUEntityListIteratorRef* iterator);

/**
@brief Retrieves the specified entity pointer from the given iterator.
@since SketchUp 2018, API 6.0
@param[in]  iterator The iterator from which to retrieve the entity.
@param[out] entity   The entity reference retrieved.
@related SUEntityListIteratorRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if iterator is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entity is NULL
- \ref SU_ERROR_NO_DATA if the iterator references an invalid entity
*/
SU_RESULT SUEntityListIteratorGetEntity(SUEntityListIteratorRef iterator, SUEntityRef* entity);

/**
@brief Increments the provided iterator.
@since SketchUp 2018, API 6.0
@param[in,out] iterator The iterator to be incremented.
@related SUEntityListIteratorRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if iterator is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE if reached the end of the collection
*/
SU_RESULT SUEntityListIteratorNext(SUEntityListIteratorRef iterator);

/**
@brief Checks if the iterator is still within range of the list.
@since SketchUp 2018, API 6.0
@param[in]  iterator The iterator to be queried.
@param[out] in_range A boolean indicating if the iterator is in range.
@related SUEntityListIteratorRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if iterator is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if in_range is NULL
- \ref SU_ERROR_OUT_OF_RANGE if the iterator is at the end of the collection
*/
SU_RESULT SUEntityListIteratorIsInRange(SUEntityListIteratorRef iterator, bool* in_range);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_ENTITY_LIST_ITERATOR_H_
